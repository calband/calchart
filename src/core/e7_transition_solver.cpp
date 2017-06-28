//
//  e7_transition_solver.cpp
//  CalChart
//
//  Created by Kevin Durand on 10/12/16.
//
//

#include "e7_transition_solver.h"
#include "munkres.h"
#include <math.h>
#include <limits>
#include <fstream>
#include <algorithm>

# pragma mark - Shared

// ==================
// ===-- SHARED --===
// ==================


class SolverCoord {
public:
    static const unsigned kFieldWidthInSteps = (100 / 5) * 8; // 100 yards, 8 steps per yard
    static const unsigned kFieldHeightInSteps = 32 + 20 + 32; // 32 steps sideline-to-hash, 20 steps hash-to-hash
    
    int32_t x;
    int32_t y;
    
    SolverCoord(CC_coord showCoord, unsigned fieldWidthInSteps = kFieldWidthInSteps, unsigned fieldHeightInSteps = kFieldHeightInSteps) : x(Coord2Float(showCoord.x) + (fieldWidthInSteps/2)), y(Coord2Float(showCoord.y) + (fieldHeightInSteps/2)) {};
    SolverCoord(int32_t x = 0, int32_t y = 0) : x(x), y(y) {};
    
    CC_coord toShowSpace(unsigned fieldWidthInSteps = kFieldWidthInSteps, unsigned fieldHeightInSteps = kFieldHeightInSteps) {
        CC_coord showCoord;
        showCoord.x = Float2Coord(((float)this->x) - (fieldWidthInSteps/2));
        showCoord.y = Float2Coord(((float)this->y) - (fieldHeightInSteps/2));
        return showCoord;
    }
    
    static SolverCoord fromShowSpace(CC_coord showCoord, unsigned fieldWidthInSteps = kFieldWidthInSteps, unsigned fieldHeightInSteps = kFieldHeightInSteps) {
        return SolverCoord(showCoord, fieldWidthInSteps, fieldHeightInSteps);
    }
    static CC_coord toShowSpace(SolverCoord solverCoord, unsigned fieldWidthInSteps = kFieldWidthInSteps, unsigned fieldHeightInSteps = kFieldHeightInSteps) {
        return solverCoord.toShowSpace(fieldWidthInSteps, fieldHeightInSteps);
    }
    
    bool operator==(const SolverCoord& other) const {
        return (this->x == other.x && this->y == other.y);
    }
    bool operator==(int32_t scalar) const {
        return (this->x == scalar && this->y == scalar);
    }
    
    SolverCoord& operator=(const SolverCoord& other) {
        this->x = other.x;
        this->y = other.y;
        return *this;
    }
    SolverCoord& operator+=(const SolverCoord& other) {
        this->x += other.x;
        this->y += other.y;
        return *this;
    }
    SolverCoord& operator-=(const SolverCoord& other) {
        this->x -= other.x;
        this->y -= other.y;
        return *this;
    }
    SolverCoord& operator*=(int32_t scalar) {
        this->x *= scalar;
        this->y *= scalar;
        return *this;
    }
    SolverCoord& operator/=(int32_t scalar) {
        this->x /= scalar;
        this->y /= scalar;
        return *this;
    }
    SolverCoord& operator+=(int32_t scalar) {
        this->x += scalar;
        this->y += scalar;
        return *this;
    }
    SolverCoord& operator-=(int32_t scalar) {
        this->x -= scalar;
        this->y -= scalar;
        return *this;
    }
    
    SolverCoord operator+(const SolverCoord& other) const {
        SolverCoord newCoord = *this;
        newCoord += other;
        return newCoord;
    }
    SolverCoord operator-(const SolverCoord& other) const {
        SolverCoord newCoord = *this;
        newCoord -= other;
        return newCoord;
    }
    SolverCoord operator*(int32_t scalar) const {
        SolverCoord newCoord = *this;
        newCoord *= scalar;
        return newCoord;
    }
    SolverCoord operator/(int32_t scalar) const {
        SolverCoord newCoord = *this;
        newCoord /= scalar;
        return newCoord;
    }
    SolverCoord operator+(int32_t scalar) const {
        SolverCoord newCoord = *this;
        newCoord += scalar;
        return newCoord;
    }
    SolverCoord operator-(int32_t scalar) const {
        SolverCoord newCoord = *this;
        newCoord -= scalar;
        return newCoord;
    }
    
    SolverCoord operator-() const {
        return (*this) * -1;
    }
    
};

struct Collision {
    unsigned beat;
    unsigned firstMarcher;
    unsigned secondMarcher;
    CC_coord position;
    
    bool operator==(const Collision &other) const
    {
        return (beat == other.beat && ((firstMarcher == other.firstMarcher && secondMarcher == other.secondMarcher) || (firstMarcher == other.secondMarcher && secondMarcher == other.firstMarcher)) && position == other.position);
    }
};

struct MovingMarcher {
    unsigned waitBeats;
    std::pair<SolverCoord, SolverCoord> stepVectors;
    std::pair<unsigned, unsigned> numSteps;
    SolverCoord startPos;
    
    bool operator==(const MovingMarcher &other) const {
        return waitBeats == other.waitBeats && stepVectors == other.stepVectors && numSteps == other.numSteps && startPos == other.startPos;
    }
};

struct MarcherMoveSegments {
    unsigned lastBeatOfWait;
    unsigned lastBeatOfFirstMove;
    unsigned lastBeatOfSecondMove;
};

struct MarcherClippedMovementState
{
    MarcherMoveSegments     moveSegments;
    SolverCoord             currentPosition;
    unsigned                currentBeat;
};

struct NotEnoughTimeException {};


class CollisionSpace {
public:
    CollisionSpace(unsigned gridXSize, unsigned gridYSize, const std::vector<SolverCoord>& marcherStartPositions, unsigned maxBeats);
private:
    // Overall state (these are accurate up to numBeats)
    unsigned m_numBeats;
    std::vector<MovingMarcher> m_marchers;
    std::set<unsigned> m_incompleteTransitions;
    
    // Grid state (all of these are accurate up to the clip beat)
    unsigned m_clipBeat;
    mutable std::vector<std::vector<std::map<unsigned, SolverCoord>>> m_collisions;
    mutable std::vector<std::vector<std::vector<std::set<unsigned>>>> m_marcherGrid;
    mutable std::vector<MarcherClippedMovementState> m_clippedMarchers;
    std::set<unsigned> m_disabledMarchers;
    
    // Cached states
    mutable bool m_cachedCollisionsNeedRefresh;
    mutable unsigned m_cachedCollisionsBeat;
    mutable std::vector<Collision> m_cachedCollisionPairs;
    mutable bool m_clipBeatCollisionsNeedRefresh;
    mutable std::vector<Collision> m_cachedCollisionsForClipBeat;
    
public:
    bool isSolved() const;
    
    void disableMarcher(unsigned which);
    void enableMarcher(unsigned which);
    
    void reinstructMarcher(unsigned which, const MovingMarcher& newMarcherAnimation);
    void clipToBeat(unsigned clipBeat);
    
    std::set<unsigned> getMarchersWithIncompleteTransitions() const;
    
    std::vector<Collision> collectCollisionPairs() const;
    std::vector<Collision> collectCollisionPairs(unsigned maxBeat) const;
    unsigned beatsBeforeCollisionBetweenMarchers(unsigned marcher1, unsigned marcher2) const;
    unsigned lastBeatContainingMovement() const;
    
    const MovingMarcher& getMarcherInstruction(unsigned marcherIndex) const;
private:
    std::vector<Collision> _collectCollisionPairs(unsigned maxBeat) const;
    
    MarcherMoveSegments clipMarcherMovement(const MarcherMoveSegments& movement, unsigned beat) const;
    MarcherMoveSegments getClippedMarcherMovement(unsigned which, unsigned beat) const;
    
    void advanceMarcher(unsigned which, unsigned numBeats);
    void rollbackMarcherInstructions(unsigned which);
    void instructMarcher(unsigned which, const MovingMarcher& newInstructions);
    
    const std::set<unsigned>& getMarchersAt(unsigned gridX, unsigned gridY, unsigned beat) const;
    std::set<unsigned>& getMarchersAt(unsigned gridX, unsigned gridY, unsigned beat);
    
    void _forgetCollision(unsigned firstMarcher, unsigned secondMarcher, unsigned beat);
    void _registerCollision(unsigned firstMarcher, unsigned secondMarcher, unsigned beat, SolverCoord pos);
    
    void forgetCollision(unsigned firstMarcher, unsigned secondMarcher, unsigned beat);
    void registerCollision(unsigned firstMarcher, unsigned secondMarcher, unsigned beat, SolverCoord pos);
    
    void removeMarcher(unsigned marcher, unsigned gridX, unsigned gridY, unsigned beat, const SolverCoord &moveVectorFromPrevBeat);
    void placeMarcher(unsigned marcher, unsigned gridX, unsigned gridY, unsigned beat, const SolverCoord &moveVectorFromPrevBeat);
    
    std::set<unsigned> getMarchersWithMovePattern(unsigned startBeat, SolverCoord firstCoord, SolverCoord secondCoord) const;
};


CollisionSpace::CollisionSpace(unsigned gridXSize, unsigned gridYSize, const std::vector<SolverCoord>& marcherStartPositions, unsigned maxBeats)
: m_numBeats(maxBeats), m_clipBeat(m_numBeats), m_cachedCollisionsNeedRefresh(true), m_cachedCollisionsBeat(0), m_clipBeatCollisionsNeedRefresh(true)
{
    m_marcherGrid.resize(gridXSize + 1);
    for (auto yGridIter = m_marcherGrid.begin(); yGridIter != m_marcherGrid.end(); yGridIter++) {
        auto& yGrid = *yGridIter;
        yGrid.resize(gridYSize + 1);
        for (auto beatsVecIter = yGrid.begin(); beatsVecIter != yGrid.end(); beatsVecIter++) {
            auto& beatsVec = *beatsVecIter;
            beatsVec.resize(maxBeats + 1);
        }
    }
    m_collisions.resize(marcherStartPositions.size());
    for (auto collisionsIter = m_collisions.begin(); collisionsIter != m_collisions.end(); collisionsIter++) {
        (*collisionsIter).resize(marcherStartPositions.size());
    }
    m_marchers.resize(marcherStartPositions.size());
    m_clippedMarchers.resize(marcherStartPositions.size());
    for (unsigned marcherIndex = 0; marcherIndex < marcherStartPositions.size(); marcherIndex++) {
        MovingMarcher marcher = {
            .waitBeats = 0,
            .stepVectors = { SolverCoord(), SolverCoord() },
            .numSteps = { 0, 0 },
            .startPos = marcherStartPositions.at(marcherIndex)
        };
        MarcherMoveSegments moveBreakdown = {
            .lastBeatOfWait = 0,
            .lastBeatOfFirstMove = 0,
            .lastBeatOfSecondMove = 0
        };
        m_clippedMarchers[marcherIndex] = {
            .moveSegments = moveBreakdown,
            .currentPosition = marcher.startPos,
            .currentBeat = 0
        };
        placeMarcher(marcherIndex, marcher.startPos.x, marcher.startPos.y, 0, SolverCoord(0,0));
        instructMarcher(marcherIndex, marcher);
    }
}

bool CollisionSpace::isSolved() const {
    return (m_clipBeat == m_numBeats) && (collectCollisionPairs().size() == 0) && (getMarchersWithIncompleteTransitions().size() == 0 && m_disabledMarchers.size() == 0);
}

std::set<unsigned> CollisionSpace::getMarchersWithIncompleteTransitions() const {
    return m_incompleteTransitions;
}

void CollisionSpace::disableMarcher(unsigned which)
{
    if (m_disabledMarchers.find(which) == m_disabledMarchers.end())
    {
        m_disabledMarchers.insert(which);
        reinstructMarcher(which, m_marchers[which]);
    }
}

void CollisionSpace::enableMarcher(unsigned which)
{
    if (m_disabledMarchers.find(which) != m_disabledMarchers.end())
    {
        m_disabledMarchers.erase(which);
        reinstructMarcher(which, m_marchers[which]);
    }
}

std::vector<Collision> CollisionSpace::collectCollisionPairs() const {
    if (m_clipBeatCollisionsNeedRefresh) {
        m_cachedCollisionsForClipBeat = _collectCollisionPairs(m_clipBeat);
        m_clipBeatCollisionsNeedRefresh = false;
    }
    return m_cachedCollisionsForClipBeat;
}

std::vector<Collision> CollisionSpace::collectCollisionPairs(unsigned maxBeat) const {
    if (maxBeat > m_clipBeat)
    {
        maxBeat = m_clipBeat;
    }
    
    if (maxBeat == m_clipBeat) {
        return collectCollisionPairs();
    }
    
    if (maxBeat != m_cachedCollisionsBeat || m_cachedCollisionsNeedRefresh)
    {
        m_cachedCollisionPairs = _collectCollisionPairs(maxBeat);
        m_cachedCollisionsBeat = maxBeat;
        m_cachedCollisionsNeedRefresh = false;
    }
    
    return m_cachedCollisionPairs;
}

std::vector<Collision> CollisionSpace::_collectCollisionPairs(unsigned maxBeat) const {
    std::vector<Collision> collisions;
    for (unsigned i = 0; i < m_marchers.size(); i++) {
        for (unsigned j = i + 1; j < m_marchers.size(); j++) {
            unsigned firstColBeat = beatsBeforeCollisionBetweenMarchers(i, j);
            if (firstColBeat <= maxBeat) {
                collisions.push_back({firstColBeat, i, j, m_collisions.at(i).at(j).at(firstColBeat).toShowSpace((unsigned)m_marcherGrid.size(), (unsigned)m_marcherGrid[0].size())});
            }
        }
    }
    return collisions;
}

unsigned CollisionSpace::beatsBeforeCollisionBetweenMarchers(unsigned marcher1, unsigned marcher2) const {
    auto& collisionsBetweenMarchers = m_collisions[marcher1][marcher2];
    unsigned firstColBeat = m_numBeats + 1;
    for (auto colIter = collisionsBetweenMarchers.begin(); colIter != collisionsBetweenMarchers.end(); colIter++) {
        if (colIter->first < firstColBeat) {
            firstColBeat = colIter->first;
        }
    }
    return firstColBeat;
}

unsigned CollisionSpace::lastBeatContainingMovement() const
{
    unsigned            maxMoveBeat = 0;
    
    for (unsigned i = 0; i < m_marchers.size(); i++)
    {
        MarcherMoveSegments     moveSegments;
        
        moveSegments = getClippedMarcherMovement(i, m_clipBeat);
        
        if (moveSegments.lastBeatOfSecondMove > maxMoveBeat)
        {
            maxMoveBeat = moveSegments.lastBeatOfSecondMove;
        }
    }
    
    return maxMoveBeat;
}

const MovingMarcher& CollisionSpace::getMarcherInstruction(unsigned marcherIndex) const {
    return m_marchers.at(marcherIndex);
}

MarcherMoveSegments CollisionSpace::clipMarcherMovement(const MarcherMoveSegments& movement, unsigned beat) const {
    MarcherMoveSegments result = {
        .lastBeatOfWait = std::min(movement.lastBeatOfWait, beat),
        .lastBeatOfFirstMove = std::min(movement.lastBeatOfFirstMove, beat),
        .lastBeatOfSecondMove = std::min(movement.lastBeatOfSecondMove, beat)
    };
    return result;
}

MarcherMoveSegments CollisionSpace::getClippedMarcherMovement(unsigned which, unsigned beat) const {
    return clipMarcherMovement(m_clippedMarchers[which].moveSegments, beat);
}

void CollisionSpace::rollbackMarcherInstructions(unsigned which) {
    auto& marcher = m_marchers[which];
    SolverCoord pos = marcher.startPos;
    unsigned beat = 0;
    
    MarcherMoveSegments marcherMovements = getClippedMarcherMovement(which, m_clippedMarchers[which].currentBeat);
    
    m_incompleteTransitions.erase(which);
    
    for (; beat <= marcherMovements.lastBeatOfWait; beat++) {
        removeMarcher(which, pos.x, pos.y, beat, SolverCoord(0,0));
    }
    for (; beat <= marcherMovements.lastBeatOfFirstMove; beat++) {
        pos += marcher.stepVectors.first;
        removeMarcher(which, pos.x, pos.y, beat, marcher.stepVectors.first);
    }
    for (; beat <= marcherMovements.lastBeatOfSecondMove; beat++) {
        pos += marcher.stepVectors.second;
        removeMarcher(which, pos.x, pos.y, beat, marcher.stepVectors.second);
    }
    for (; beat <= m_clippedMarchers[which].currentBeat; beat++) {
        removeMarcher(which, pos.x, pos.y, beat, SolverCoord(0,0));
    }
    
    // After a rollback, we still want our marcher in its first position
    placeMarcher(which, marcher.startPos.x, marcher.startPos.y, 0, SolverCoord(0,0));
    m_clippedMarchers[which].currentBeat = 0;
    m_clippedMarchers[which].currentPosition = marcher.startPos;
}

void CollisionSpace::instructMarcher(unsigned which, const MovingMarcher& newInstructions) {
    m_marchers[which] = newInstructions;
    auto& marcher = m_marchers[which];
    
    MarcherClippedMovementState &clippedMarcher = m_clippedMarchers[which];
    
    clippedMarcher.moveSegments.lastBeatOfWait = marcher.waitBeats;
    clippedMarcher.moveSegments.lastBeatOfFirstMove = clippedMarcher.moveSegments.lastBeatOfWait + marcher.numSteps.first;
    clippedMarcher.moveSegments.lastBeatOfSecondMove = clippedMarcher.moveSegments.lastBeatOfFirstMove + marcher.numSteps.second;
    
    if (clippedMarcher.moveSegments.lastBeatOfSecondMove > m_numBeats)
    {
        m_incompleteTransitions.insert(which);
    }
    
    advanceMarcher(which, m_clipBeat);
}

void CollisionSpace::advanceMarcher(unsigned which, unsigned numBeats) {
    auto& marcher = m_marchers[which];
    auto moveSegments = getClippedMarcherMovement(which, m_clipBeat);
    
    MarcherClippedMovementState &clippedMarcher = m_clippedMarchers[which];
    SolverCoord pos = clippedMarcher.currentPosition;
    unsigned beat = clippedMarcher.currentBeat + 1;
    
    for (; beat <= moveSegments.lastBeatOfWait && numBeats > 0; beat++, numBeats--) {
        placeMarcher(which, pos.x, pos.y, beat, SolverCoord(0,0));
    }
    for (; beat <= moveSegments.lastBeatOfFirstMove && numBeats > 0; beat++, numBeats--) {
        pos += marcher.stepVectors.first;
        placeMarcher(which, pos.x, pos.y, beat, marcher.stepVectors.first);
    }
    for (; beat <= moveSegments.lastBeatOfSecondMove && numBeats > 0; beat++, numBeats--) {
        pos += marcher.stepVectors.second;
        placeMarcher(which, pos.x, pos.y, beat, marcher.stepVectors.second);
    }
    for (; beat <= m_clipBeat && numBeats > 0; beat++, numBeats--) {
        placeMarcher(which, pos.x, pos.y, beat, SolverCoord(0,0));
    }
    
    clippedMarcher.currentPosition = pos;
    clippedMarcher.currentBeat = beat - 1;
}


void CollisionSpace::reinstructMarcher(unsigned which, const MovingMarcher& newMarcherAnimation) {
    m_clipBeatCollisionsNeedRefresh = true;
    m_cachedCollisionsNeedRefresh = true;
    
    rollbackMarcherInstructions(which);
    
    if (m_disabledMarchers.find(which) != m_disabledMarchers.end())
    {
        m_marchers[which] = newMarcherAnimation;
    } else {
        instructMarcher(which, newMarcherAnimation);
    }
}

void CollisionSpace::clipToBeat(unsigned clipBeat) {
    if (clipBeat > m_numBeats)
    {
        clipBeat = m_numBeats;
    }
    
    if (clipBeat > m_clipBeat) {
        for (unsigned i = 0; i < m_marchers.size(); i++)
        {
            advanceMarcher(i, clipBeat - m_clipBeat);
        }
        
    } else {
        for (unsigned i = 0; i < m_marchers.size(); i++) {
            reinstructMarcher(i, m_marchers[i]);
        }
    }
    
    if (clipBeat != m_clipBeat) {
        if (m_clipBeat == m_cachedCollisionsBeat && !m_cachedCollisionsNeedRefresh) {
            m_cachedCollisionsForClipBeat = m_cachedCollisionPairs;
        } else {
            // Require caches to update
            m_clipBeatCollisionsNeedRefresh = true;
            m_cachedCollisionsNeedRefresh = true;
        }
    }
    
    m_clipBeat = clipBeat;
}


const std::set<unsigned>& CollisionSpace::getMarchersAt(unsigned gridX, unsigned gridY, unsigned beat) const {
    return m_marcherGrid.at(gridX).at(gridY).at(beat);
}

std::set<unsigned>& CollisionSpace::getMarchersAt(unsigned gridX, unsigned gridY, unsigned beat) {
    return m_marcherGrid[gridX][gridY][beat];
}

void CollisionSpace::_forgetCollision(unsigned int firstMarcher, unsigned int secondMarcher, unsigned int beat) {
    auto& collisionsBetweenThem = m_collisions[firstMarcher][secondMarcher];
    auto theCollision = collisionsBetweenThem.find(beat);
    if (theCollision != collisionsBetweenThem.end()) {
        collisionsBetweenThem.erase(theCollision);
    }
    if (firstMarcher < secondMarcher) {
        _forgetCollision(secondMarcher, firstMarcher, beat);
    }
}

void CollisionSpace::_registerCollision(unsigned int firstMarcher, unsigned int secondMarcher, unsigned int beat, SolverCoord pos) {
    m_collisions[firstMarcher][secondMarcher][beat] = pos;
    if (firstMarcher < secondMarcher) {
        _registerCollision(secondMarcher, firstMarcher, beat, pos);
    }
}

void CollisionSpace::forgetCollision(unsigned int firstMarcher, unsigned int secondMarcher, unsigned int beat) {
    if (firstMarcher < secondMarcher) {
        _forgetCollision(firstMarcher, secondMarcher, beat);
    } else {
        _forgetCollision(secondMarcher, firstMarcher, beat);
    }
}

void CollisionSpace::registerCollision(unsigned int firstMarcher, unsigned int secondMarcher, unsigned int beat, SolverCoord pos) {
    if (firstMarcher < secondMarcher) {
        _registerCollision(firstMarcher, secondMarcher, beat, pos);
    } else {
        _registerCollision(secondMarcher, firstMarcher, beat, pos);
    }
}

void CollisionSpace::removeMarcher(unsigned marcher, unsigned gridX, unsigned gridY, unsigned beat, const SolverCoord &moveVectorFromPrevBeat) {
    auto& marchersAtGridLoc = getMarchersAt(gridX, gridY, beat);
    
    marchersAtGridLoc.erase(marcher);
    for (auto otherMarcher : marchersAtGridLoc) {
        forgetCollision(marcher, otherMarcher, beat);
    }
    
    // Retract collisions resulting from swaps
    if (beat > 0 && !(moveVectorFromPrevBeat == 0))
    {
        SolverCoord thisLocation((int32_t)gridX, (int32_t)gridY);
        auto swappingMarchers = getMarchersWithMovePattern(beat - 1, thisLocation, thisLocation -  moveVectorFromPrevBeat);
        for (auto otherMarcher : swappingMarchers) {
            forgetCollision(marcher, otherMarcher, beat);
        }
    }
}

void CollisionSpace::placeMarcher(unsigned marcher, unsigned gridX, unsigned gridY, unsigned beat, const SolverCoord &moveVectorFromPrevBeat) {
    SolverCoord thisLocation((int32_t)gridX, (int32_t)gridY);
    auto& marchersAtGridLoc = getMarchersAt(gridX, gridY, beat);
    for (auto otherMarcher : marchersAtGridLoc) {
        registerCollision(marcher, otherMarcher, beat, CC_coord(gridX, gridY));
    }
    
    // Add collisions resulting from swaps
    if (beat > 0 && !(moveVectorFromPrevBeat == 0))
    {
        SolverCoord thisLocation((int32_t)gridX, (int32_t)gridY);
        auto swappingMarchers = getMarchersWithMovePattern(beat - 1, thisLocation, thisLocation - moveVectorFromPrevBeat);
        for (auto otherMarcher : swappingMarchers) {
            registerCollision(marcher, otherMarcher, beat, /*CC_coord(gridX, gridY)*/ CC_coord(0,0));
        }
    }
    
    
    marchersAtGridLoc.insert(marcher);
}

std::set<unsigned> CollisionSpace::getMarchersWithMovePattern(unsigned startBeat, SolverCoord firstCoord, SolverCoord secondCoord) const
{
    std::set<unsigned>   result;
    
    auto& marchersAtFirstCoord = getMarchersAt(firstCoord.x, firstCoord.y, startBeat);
    auto& marchersAtSecondCoord = getMarchersAt(secondCoord.x, secondCoord.y, startBeat + 1);
    
    for (unsigned i : marchersAtFirstCoord) {
        if (marchersAtSecondCoord.find(i) != marchersAtSecondCoord.end()) {
            result.insert(i);
        }
    }
    
    return result;
}

void fillPositionsFromSheet(const CC_sheet& sheet, std::vector<SolverCoord>& positions) {
    auto points = sheet.GetPoints();
    for (size_t i = 0; i < points.size(); i++) {
        positions.push_back(SolverCoord::fromShowSpace(points[i].GetPos()));
    }
}

Matrix<double> makeHungarianDistanceMatrix(const std::vector<SolverCoord>& startPositions, const std::vector<SolverCoord>& endPositions, unsigned numBeats) {
    size_t numMarchers = startPositions.size();
    Matrix<double> matrix = Matrix<double>(numMarchers, numMarchers);
    for (size_t i = 0; i < endPositions.size(); i++) {
        for (size_t j = 0; j < startPositions.size(); j++) {
            auto diff = startPositions.at(j) - endPositions.at(i);
            unsigned manhattanDist = abs(diff.x) + abs(diff.y);
            if (manhattanDist > numBeats) {
                matrix(j, i) = std::numeric_limits<double>::max();//std::numeric_limits<double>::infinity();
            } else {
                matrix(j, i) = manhattanDist + 1;
            }
        }
    }
    return matrix;
}

enum PathInstruction
{
    PATH_DEGENERATE_STRAIGHT = 0, // A straight-line path; can be replaced with any other instruction type
    PATH_EWNS,
    PATH_NSEW,
    PATH_DEGENERATE_DIAGONAL, // A degenerate diagonal path; can be replaced by DMHS or HSDM
    PATH_DMHS,
    PATH_HSDM,
    PATH_INVALID
};

enum CompassDirection
{
    COMPASS_N = 0,
    COMPASS_NE,
    COMPASS_E,
    COMPASS_SE,
    COMPASS_S,
    COMPASS_SW,
    COMPASS_W,
    COMPASS_NW,
    COMPASS_INVALID
};

const SolverCoord kZeroVector(0, 0);
const SolverCoord kNVector(1, 0);
const SolverCoord kEVector(0, 1);
const SolverCoord kNEVector(1, 1);
const SolverCoord kSEVector(1, -1);

const SolverCoord kMoveVectorsByDirection[COMPASS_INVALID + 1] = {
    /* kMoveVectorsByDirection[COMPASS_N] = */          kNVector,
    /* kMoveVectorsByDirection[COMPASS_NE] = */         kNEVector,
    /* kMoveVectorsByDirection[COMPASS_E] = */          kEVector,
    /* kMoveVectorsByDirection[COMPASS_SE] = */         kSEVector,
    /* kMoveVectorsByDirection[COMPASS_S] = */          -kNVector,
    /* kMoveVectorsByDirection[COMPASS_SW] = */         -kNEVector,
    /* kMoveVectorsByDirection[COMPASS_W] = */          -kEVector,
    /* kMoveVectorsByDirection[COMPASS_NW] = */         -kSEVector
    /* kMoveVectorsByDirection[COMPASS_INVALID] = */    -kZeroVector
};

const CompassDirection kDirectionByMoveVector[3][3] = {
    { COMPASS_SW, COMPASS_S, COMPASS_SE},
    { COMPASS_W, COMPASS_INVALID, COMPASS_E},
    { COMPASS_NW, COMPASS_N, COMPASS_NE }
};

struct PathAxes {
    CompassDirection majorAxis = COMPASS_INVALID;
    CompassDirection minorAxis = COMPASS_INVALID;
};


struct MarcherSolution {
    SolverCoord startPos;
    SolverCoord endPos;
    unsigned waitBeats;
    PathInstruction instruction;
};

struct MarcherSolverState {
    PathAxes pathAxes;
    std::pair<SolverCoord, SolverCoord> stepVectors;
    std::pair<unsigned, unsigned> numSteps;
};

PathAxes calcPathAxes(SolverCoord startPos, SolverCoord endPos) {
    PathAxes result;
    SolverCoord diff = endPos - startPos;
    int16_t absDiffX = abs(diff.x);
    int16_t absDiffY = abs(diff.y);
    if (diff.x) {
        diff.x /= absDiffX;
    }
    if (diff.y) {
        diff.y /= absDiffY;
    }
    if (absDiffX > absDiffY) {
        result.majorAxis = kDirectionByMoveVector[diff.x + 1][1];
        result.minorAxis = kDirectionByMoveVector[1][diff.y + 1];
    } else {
        result.majorAxis = kDirectionByMoveVector[1][diff.y + 1];
        result.minorAxis = kDirectionByMoveVector[diff.x + 1][1];
    }
    return result;
}

std::pair<SolverCoord, SolverCoord> calcStepVectors(PathAxes axes, PathInstruction instruction) {
    std::pair<SolverCoord, SolverCoord> stepVectors;
    bool majorStepIsFirst = false;
    SolverCoord majorStep = kMoveVectorsByDirection[axes.majorAxis];
    SolverCoord minorStep = kMoveVectorsByDirection[axes.minorAxis];
    switch (instruction) {
        case PATH_DEGENERATE_STRAIGHT:
            majorStepIsFirst = true;
        case PATH_EWNS:
            majorStepIsFirst = !(minorStep.y != 0);
            break;
        case PATH_NSEW:
            majorStepIsFirst = !(minorStep.x != 0);
            break;
        case PATH_DEGENERATE_DIAGONAL:
        case PATH_HSDM:
            majorStepIsFirst = true;
        case PATH_DMHS:
            minorStep += majorStep;
            break;
        default:
            break;
    }
    if (majorStepIsFirst) {
        stepVectors.first = majorStep;
        stepVectors.second = minorStep;
    } else {
        stepVectors.first = minorStep;
        stepVectors.second = majorStep;
    }
    
    if (stepVectors.first == stepVectors.second)
    {
        stepVectors.second = 0;
    }
    
    return stepVectors;
}

std::pair<float, float> calcStepsInEachDir(SolverCoord startPos, SolverCoord endPos, SolverCoord stepVector1, SolverCoord stepVector2) {
    // In this function, we ASSUME that stepVector1 and stepVector2 are orthogonal -- if this isn't true our calculations will be bogus

    // Make sure that the first vector has an x-component, since we'll be dividing by it later
    // We know at least one of the vectors will have an x-component, since they must be orthogonal
    SolverCoord vec1, vec2;
    bool swap = (stepVector1.x == 0);
    if (swap) {
        vec1 = stepVector2;
        vec2 = stepVector1;
    } else {
        vec1 = stepVector1;
        vec2 = stepVector2;
    }
    
    // startPos + (numStepsLeg1 * stepVector1) + (numStepsLeg2 * stepVector2) = endPos
    // (numStepsLeg1 * stepVector1) + (numStepsLeg2 * stepVector2) = endPos - startPos = moveVector
    SolverCoord moveVector = endPos - startPos;
    
    float       numSteps1;
    float       numSteps2;
    
    if (vec1.x != 0)
    {
        // Split the equation above into one equation for each vector component (one for x components and one for y)
        // Focus first on the x components
        // (numStepsLeg1 * stepVector1.x) + (numStepsLeg2 * stepVector2.x) = moveVector.x
        // (numStepsLeg1 * stepVector1.x) = moveVector.x - (numStepsLeg2 * stepVector2.x)
        // numStepsLeg1 = (moveVector.x / stepVector1.x) - (stepVector2.x / stepVector1.x) * numStepsLeg2 = a - b * numStepsLeg2
        float a = ((float)(moveVector.x)) / vec1.x;
        float b = ((float)(vec2.x)) / vec1.x;

        // Next consider y components
        // (numStepsLeg1 * stepVector1.y) + (numStepsLeg2 * stepVector2.y) = moveVector.y
        // ((a - b * numStepsLeg2) * stepVector1.y) + (numStepsLeg2 * stepVector2.y) = moveVector.y
        // (a * stepVector1.y) - (numStepsLeg2 * b * stepVector1.y) + (numStepsLeg2 * stepVector2.y) = moveVector.y
        // numStepsLeg2 * (stepVector2.y - b * stepVector1.y) = moveVector.y - (a * stepVector1.y)
        // numStepsLeg2 = (moveVector.y - (a * stepVector1.y)) / (stepVector2.y - (b * stepVector1.y))
        if (vec2.x == 0 && vec2.y == 0)
        {
            numSteps2 = 0;
        }
        else
        {
            numSteps2 = (moveVector.y - (a * vec1.y)) / (vec2.y - (b * vec1.y));
        }
        
        numSteps1 = a - (b * numSteps2);
    }
    else
    {
        if (vec1.y != 0)
        {
            numSteps1 = moveVector.y / vec1.y;
            numSteps2 = 0;
        }
        else if (vec2.y != 0)
        {
            numSteps1 = 0;
            numSteps2 = moveVector.y / vec2.y;
        }
        else
        {
            numSteps1 = 0;
            numSteps2 = 0;
        }
    }
    
    if (swap) {
        return { numSteps2, numSteps1 };
    } else {
        return { numSteps1, numSteps2 };
    }
}

void setupMarcherSolutionPath(MarcherSolution& marcherSolution, const TransitionSolverParams::InstructionOption &instruction, SolverCoord newDestination)
{
    marcherSolution.endPos = newDestination;
    marcherSolution.waitBeats = instruction.waitBeats;
    switch (instruction.movementPattern)
    {
        case TransitionSolverParams::InstructionOption::Pattern::EWNS:
            marcherSolution.instruction = PATH_EWNS;
            break;
        case TransitionSolverParams::InstructionOption::Pattern::NSEW:
            marcherSolution.instruction = PATH_NSEW;
            break;
        case TransitionSolverParams::InstructionOption::Pattern::DMHS:
            marcherSolution.instruction = PATH_DMHS;
            break;
        case TransitionSolverParams::InstructionOption::Pattern::HSDM:
            marcherSolution.instruction = PATH_HSDM;
            break;
        default:
            break;
    }
}

MovingMarcher calculateMovementFromSolution(const MarcherSolution& marcher)
{
    PathAxes pathAxes = calcPathAxes(marcher.startPos, marcher.endPos);
    std::pair<SolverCoord, SolverCoord> stepVectors = calcStepVectors(pathAxes, marcher.instruction);
    std::pair<float, float> numSteps = calcStepsInEachDir(marcher.startPos, marcher.endPos, stepVectors.first, stepVectors.second);
    
    MovingMarcher marcherAnim;
    marcherAnim.waitBeats = marcher.waitBeats;
    marcherAnim.stepVectors = stepVectors;
    marcherAnim.numSteps = numSteps;
    marcherAnim.startPos = marcher.startPos;
    
    return marcherAnim;
}

void refreshMarcherPathInCollisionSpace(CollisionSpace& colSpace, unsigned marcherIndex, const MarcherSolution& marcherSol)
{
    MovingMarcher marcherAnim = calculateMovementFromSolution(marcherSol);
    
    colSpace.reinstructMarcher(marcherIndex, marcherAnim);
}

class FileReader
{
public:
    static std::vector<SolverCoord> readInitialFile(std::string filepath);
    static std::vector<SolverCoord> readFinalFile(std::string filepath);
private:
};

std::vector<SolverCoord> FileReader::readInitialFile(std::string filepath)
{
    std::map<unsigned, SolverCoord>     positionsByIndex;
    std::vector<SolverCoord>            positions;
    std::ifstream                       file(filepath);
    
    if (file.is_open())
    {
        int32_t                                 y = 0;
        uint32_t                                maxMarcherIndex = 0;
        std::string                             currLine;
        
        while (getline(file, currLine))
        {
            int32_t                             x = 0;
            int32_t                             gridOccupant;
            std::istringstream                  lineStream(currLine);
            std::istream_iterator<std::string>  wordsBegin(lineStream), wordsEnd;
            std::vector<std::string>            rowOccupants(wordsBegin, wordsEnd);
            
            for (unsigned i = 0; i < rowOccupants.size(); i++)
            {
                gridOccupant = std::stoi(rowOccupants[i]);
                
                if (gridOccupant > 0)
                {
                    positionsByIndex[gridOccupant] = SolverCoord(x, y);
                    
                    if ((uint32_t)gridOccupant > maxMarcherIndex)
                    {
                        maxMarcherIndex = gridOccupant;
                    }
                }
                
                x+=2;
            }
            
            y+=2;
        }
        
        for (unsigned marcherIndex = 0; marcherIndex < maxMarcherIndex; marcherIndex++)
        {
            positions.push_back(positionsByIndex[marcherIndex + 1]);
        }
    }
    
    return positions;
}

std::vector<SolverCoord> FileReader::readFinalFile(std::string filepath)
{
    std::map<unsigned, SolverCoord>     positionsByIndex;
    std::vector<SolverCoord>            positions;
    std::string                         currLine;
    std::ifstream                       file(filepath);
    
    if (file.is_open())
    {
        int32_t                                 y = 0;
        std::string                             currLine;
        
        while (getline(file, currLine))
        {
            int32_t                             x = 0;
            bool                                hasGridOccupant;
            std::istringstream                  lineStream(currLine);
            std::istream_iterator<std::string>  wordsBegin(lineStream), wordsEnd;
            std::vector<std::string>            rowOccupants(wordsBegin, wordsEnd);
            
            for (unsigned i = 0; i < rowOccupants.size(); i++)
            {
                hasGridOccupant = (std::stoi(rowOccupants[i]) > 0);
                
                if (hasGridOccupant)
                {
                    positions.push_back(SolverCoord(x, y));
                }
                
                x+=2;
            }
            
            y+=2;
        }
    }
    
    std::sort(positions.begin(), positions.end(),
              [] (SolverCoord const& a, SolverCoord const& b) {
                  if (a.x < b.x)
                  {
                      return true;
                  }
                  else if (a.x > b.x)
                  {
                      return false;
                  }
                  else
                  {
                      return a.y < b.y;
                  }});
    
    return positions;
}


class DestinationConstraints
{
public:
    DestinationConstraints(const std::vector<TransitionSolverParams::GroupParams> &groupConstraints, const std::vector<SolverCoord> &destinations);
    
    bool destinationIsAllowed(unsigned marcher, unsigned destination) const;
    bool destinationIsAllowed(unsigned marcher, SolverCoord destination) const;
    
private:
    class SolverCoordCompare {
    public:
        bool operator()(const SolverCoord &first, const SolverCoord &second) const {
            bool result;
            if (first.x == second.x) {
                result = (first.y < second.y);
            } else {
                result = (first.x < second.x);
            }
            return result;
        }
    };
    
    std::map<unsigned, std::set<unsigned>> m_allowedDestinations;
    std::map<SolverCoord, unsigned, SolverCoordCompare> m_destinationPositionsToIndices;
};


DestinationConstraints::DestinationConstraints(const std::vector<TransitionSolverParams::GroupParams> &groupConstraints, const std::vector<SolverCoord> &destinations)
{
    for (TransitionSolverParams::GroupParams group : groupConstraints) {
        for (auto marcher : group.marchers)
        {
            for (auto destination : group.allowedDestinations)
            {
                m_allowedDestinations[marcher].insert(destination);
            }
        }
    }
    for (unsigned i = 0; i < destinations.size(); i++)
    {
        m_destinationPositionsToIndices[destinations[i]] = i;
    }
}

bool DestinationConstraints::destinationIsAllowed(unsigned marcher, unsigned destination) const
{
    bool            result = true;
    
    if (m_allowedDestinations.find(marcher) != m_allowedDestinations.end())
    {
        const std::set<unsigned>    &allowedDestinations = m_allowedDestinations.at(marcher);
        
        result = allowedDestinations.find(destination) != allowedDestinations.end();
    }
    
    return result;
}

bool DestinationConstraints::destinationIsAllowed(unsigned marcher, SolverCoord destination) const
{
    return destinationIsAllowed(marcher, m_destinationPositionsToIndices.at(destination));
}

# pragma mark - Algorithm By: Chiu Zamora Malani

// ==============================================
// ===-- ALGORITHM BY: CHIU, ZAMORA, MALANI --===
// ==============================================

namespace e7ChiuZamoraMalani {

// First: swapLDirections, first player
// Second: swapLDirections, second player
// Third: numWaits, first player
// Fourth: numWaits, second player
// Fifth: swapPositions
struct SolutionAdjustmentInstruction {
    unsigned instructionForMarcher1;
    unsigned instructionForMarcher2;
    bool swapMarchers;
};

struct AdjustmentInstructionSorter {
    
    AdjustmentInstructionSorter(const std::vector<TransitionSolverParams::InstructionOption> &instructionOptions) : instructionOptions(instructionOptions) {};
    
    const std::vector<TransitionSolverParams::InstructionOption> instructionOptions;
    
    bool operator()(const SolutionAdjustmentInstruction &first,const SolutionAdjustmentInstruction &second) {
        unsigned                                                waitBeats1;
        unsigned                                                waitBeats2;
        unsigned                                                marchPattern1;
        unsigned                                                marchPattern2;
        
        waitBeats1 = instructionOptions.at(first.instructionForMarcher1).waitBeats + instructionOptions.at(first.instructionForMarcher2).waitBeats;
        waitBeats2 = instructionOptions.at(second.instructionForMarcher1).waitBeats + instructionOptions.at(second.instructionForMarcher2).waitBeats;
        
        marchPattern1 = (unsigned)instructionOptions[first.instructionForMarcher1].movementPattern + (unsigned)instructionOptions[first.instructionForMarcher2].movementPattern;
        marchPattern2 = (unsigned)instructionOptions[second.instructionForMarcher1].movementPattern + (unsigned)instructionOptions[second.instructionForMarcher2].movementPattern;
        
        if (waitBeats1 == waitBeats2) {
            if (marchPattern1 == marchPattern2) {
                return !first.swapMarchers;
            } else {
                return marchPattern1 < marchPattern2;
            }
        } else {
            return waitBeats1 < waitBeats2;
        }
    }
};

std::vector<SolutionAdjustmentInstruction> unfilteredAdjustmentOptions(const std::vector<TransitionSolverParams::InstructionOption> &instructionOptions)
{
    std::vector<SolutionAdjustmentInstruction> allOptions;
    for (unsigned inst1 = 0; inst1 < instructionOptions.size(); inst1++) {
        for (unsigned inst2 = 0; inst2 < instructionOptions.size(); inst2++) {
            for (bool swapPositions : {false, true}) {
                allOptions.push_back({inst1, inst2, swapPositions});
            }
        }
    }
    std::sort(allOptions.begin(), allOptions.end(), AdjustmentInstructionSorter(instructionOptions));
    return allOptions;
}

void recalculateMarcher(MovingMarcher& marcher,MarcherSolution& marcherSolution, const TransitionSolverParams::InstructionOption &instruction, SolverCoord newDestination)
{
    setupMarcherSolutionPath(marcherSolution, instruction, newDestination);
    
    marcher = calculateMovementFromSolution(marcherSolution);
}
    

void iterateSolution(std::vector<MarcherSolution>& marcherSolutions, CollisionSpace& collisionSpace, unsigned maxBeats, const  DestinationConstraints &destinationConstraints, const std::vector<TransitionSolverParams::InstructionOption> &instructionOptions, TransitionSolverDelegate *delegate)
{
    const std::vector<SolutionAdjustmentInstruction> unfilteredOptions = unfilteredAdjustmentOptions(instructionOptions);
    
    std::vector<Collision> collisionPairs;
    std::vector<bool> degeneratePaths;
    std::map<std::pair<unsigned, unsigned>, unsigned> allActiveOptions;
    
    for (unsigned i = 0; i < marcherSolutions.size(); i++) {
        SolverCoord marcherMoveDiff = marcherSolutions[i].endPos - marcherSolutions[i].startPos;
        degeneratePaths.push_back(marcherMoveDiff.x == 0 || marcherMoveDiff.y == 0);
    }
    
    bool progress = true;
    unsigned originalNumCollisions = (unsigned)collisionSpace.collectCollisionPairs().size();
    unsigned leastNumCollisions = originalNumCollisions;
    unsigned numIterationsWithoutOverallImprovement = 0;
    while (!collisionSpace.isSolved() && progress && numIterationsWithoutOverallImprovement < 10) {
        
        progress = false;
        if (delegate)
        {
            double progress;
            
            if (originalNumCollisions > collisionSpace.collectCollisionPairs().size()) {
                progress = (double)(originalNumCollisions - collisionSpace.collectCollisionPairs().size()) / (double)originalNumCollisions;
            } else {
                progress = 1;
            }
            
            delegate->OnSubtaskProgress(progress);
            
            if (delegate->ShouldAbortCalculation())
            {
                break;
            }
        }
        
        for (unsigned beat = 0; beat <= maxBeats; beat++) {
            
            collisionSpace.clipToBeat(beat);
            collisionPairs = collisionSpace.collectCollisionPairs(beat);
            
            for (unsigned collisionIndex = 0; collisionIndex < collisionPairs.size(); collisionIndex++) {
                Collision& col = collisionPairs[collisionIndex];
                
                // For each pair of colliding marchers, select the appropriate set of options for solving the collision
                const std::vector<SolutionAdjustmentInstruction>* adjustmentOptionsPtr;
                adjustmentOptionsPtr = &unfilteredOptions;
                const std::vector<SolutionAdjustmentInstruction>& adjustmentOptions = *adjustmentOptionsPtr;
                
                // Find the most recent option that was used to fix the collision; start from there when figuring out what to do next with them
                unsigned activeOptionIndex = allActiveOptions[{col.firstMarcher, col.secondMarcher}];
                SolutionAdjustmentInstruction activeOption = adjustmentOptions.at(activeOptionIndex);
                
                // Keep working on the collision until you find a solution
                MarcherSolution mutableMarcherSolution1 = marcherSolutions[col.firstMarcher];
                MarcherSolution mutableMarcherSolution2 = marcherSolutions[col.secondMarcher];
                for (unsigned newOptionIndex = activeOptionIndex + 1;; newOptionIndex++) {
                    
                    if (newOptionIndex >= adjustmentOptions.size()) {
                        newOptionIndex = 0;
                    }
                    
                    SolutionAdjustmentInstruction newOption = adjustmentOptions.at(newOptionIndex);
                    
                    MovingMarcher marcherMove1 = collisionSpace.getMarcherInstruction(col.firstMarcher);
                    MovingMarcher marcherMove2 = collisionSpace.getMarcherInstruction(col.secondMarcher);
                    
                    bool badSolutionDuration = false;
                    bool badSolutionDestination = false;
                    
                    SolverCoord destination1 = mutableMarcherSolution1.endPos;
                    SolverCoord destination2 = mutableMarcherSolution2.endPos;
                    if (newOption.swapMarchers != activeOption.swapMarchers)
                    {
                        SolverCoord tmp = destination1;
                        destination1 = destination2;
                        destination2 = tmp;
                    }
                    
                    recalculateMarcher(marcherMove1, mutableMarcherSolution1, instructionOptions[newOption.instructionForMarcher1], destination1);
                    recalculateMarcher(marcherMove2, mutableMarcherSolution2, instructionOptions[newOption.instructionForMarcher2], destination2);
                    badSolutionDuration = (marcherMove1.waitBeats + marcherMove1.numSteps.first + marcherMove1.numSteps.second > maxBeats) || (marcherMove2.waitBeats + marcherMove2.numSteps.first + marcherMove2.numSteps.second > maxBeats);
                    badSolutionDestination = !destinationConstraints.destinationIsAllowed(col.firstMarcher, mutableMarcherSolution1.endPos) || !destinationConstraints.destinationIsAllowed(col.secondMarcher, mutableMarcherSolution2.endPos);
                    
                    activeOption = newOption;
                    if ((badSolutionDuration || badSolutionDestination) && newOptionIndex != activeOptionIndex)
                    {
                        continue;
                    }
                    
                    collisionSpace.reinstructMarcher(col.firstMarcher, marcherMove1);
                    collisionSpace.reinstructMarcher(col.secondMarcher, marcherMove2);

                    allActiveOptions[{col.firstMarcher, col.secondMarcher}] = newOptionIndex;
                    marcherSolutions[col.firstMarcher] = mutableMarcherSolution1;
                    marcherSolutions[col.secondMarcher] = mutableMarcherSolution2;
                    
                    // If the collision was solved, go ahead and break from this
                    unsigned beatsWithoutCollision = collisionSpace.beatsBeforeCollisionBetweenMarchers(col.firstMarcher, col.secondMarcher);
                    if (beatsWithoutCollision > beat) {
                        progress  = true;
                        break;
                    }
                    if (newOptionIndex == activeOptionIndex) {
                        break;
                    }
                }
            }
        }
        
        unsigned numCollisions = (unsigned)collisionSpace.collectCollisionPairs().size();
        if (numCollisions < leastNumCollisions) {
            numIterationsWithoutOverallImprovement = 0;
            leastNumCollisions = numCollisions;
        } else {
            numIterationsWithoutOverallImprovement++;
        }
    }
}
    

}

# pragma mark - Algorithm By: Namini Asl, Ramirez, Zhang

// ===================================================
// ===-- ALGORITHM BY: NAMINI ASL, RAMIREZ, ZHANG --===
// ===================================================

namespace e7NaminiaslRamirezZhang {
    // Very similar to ChiuZamoraMalani
    // For each beat, try to solve all collisions by looking at each marcher and trading out options
    // However, the difference is: We try to get rid of ALL collisions on any given beat, instead of looping back to the beginning at the end (we have a max number of times we do this)
    // Second, we look through ALL options for each marcher, and choose the one that offers the LEAST number of collisions
using namespace e7ChiuZamoraMalani;

void iterateSolution(std::vector<MarcherSolution>& marcherSolutions, CollisionSpace& collisionSpace, unsigned maxBeats, const  DestinationConstraints &destinationConstraints, const std::vector<TransitionSolverParams::InstructionOption> &instructionOptions, TransitionSolverDelegate *delegate)
{
    const std::vector<SolutionAdjustmentInstruction> unfilteredOptions = unfilteredAdjustmentOptions(instructionOptions);
    
    std::vector<Collision> collisionPairs;
    std::vector<Collision> lastCollisionPairs;
    std::map<std::pair<unsigned, unsigned>, unsigned> allActiveOptions;
    
    for (unsigned beat = 1; beat <= maxBeats; beat += std::min((unsigned)1, maxBeats - beat + 1)) {
        
        if (delegate)
        {
            delegate->OnSubtaskProgress((double)beat / (double)maxBeats);
            if (delegate->ShouldAbortCalculation())
            {
                break;
            }
        }
        
        collisionSpace.clipToBeat(beat);
        
        for (unsigned improvementIteration = 0; improvementIteration < 3 || beat == maxBeats; improvementIteration++)
        {
            
            collisionPairs = collisionSpace.collectCollisionPairs();

            if (collisionPairs == lastCollisionPairs)
            {
                break;
            }
//            bool matched;
//            for (unsigned iPlusOne = collisionPairs.size(); iPlusOne > 0; iPlusOne--)
//            {
//                matched = false;
//                
//                for (Collision col : lastCollisionPairs)
//                {
//                    if (collisionPairs[iPlusOne - 1] == col)
//                    {
//                        matched = true;
//                        break;
//                    }
//                }
//                
//                if (!matched)
//                {
//                    break;
//                }
//            }
//            if (matched)
//            {
//                break;
//            }
            
            lastCollisionPairs = collisionSpace.collectCollisionPairs();
            
            std::random_shuffle(collisionPairs.begin(), collisionPairs.end());
            for (unsigned collisionIndex = 0; collisionIndex < collisionPairs.size(); collisionIndex++) {
                
                Collision& col = collisionPairs[collisionIndex];
                
                // For each pair of colliding marchers, select the appropriate set of options for solving the collision
                const std::vector<SolutionAdjustmentInstruction>* adjustmentOptionsPtr;
                adjustmentOptionsPtr = &unfilteredOptions;
                const std::vector<SolutionAdjustmentInstruction>& adjustmentOptions = *adjustmentOptionsPtr;
                
                // Find the most recent option that was used to fix the collision; start from there when figuring out what to do next with them
                unsigned activeOptionIndex = allActiveOptions[{col.firstMarcher, col.secondMarcher}];
                SolutionAdjustmentInstruction activeOption = adjustmentOptions.at(activeOptionIndex);
                unsigned bestOptionIndex = activeOptionIndex;
                unsigned bestNumCollisions = (unsigned)collisionSpace.collectCollisionPairs().size();
                
                // Check through all fix options, and find the one which makes the most improvement
                MarcherSolution mutableMarcherSolution1 = marcherSolutions[col.firstMarcher];
                MarcherSolution mutableMarcherSolution2 = marcherSolutions[col.secondMarcher];
                for (unsigned newOptionIndex = 0;; newOptionIndex++) {
                    bool executingFinalIteration;
                    
                    executingFinalIteration = (newOptionIndex == adjustmentOptions.size());
                    if (executingFinalIteration)
                    {
                        newOptionIndex = bestOptionIndex;
                    }
                    
                    SolutionAdjustmentInstruction newOption = adjustmentOptions.at(newOptionIndex);
                    
                    MovingMarcher marcherMove1 = collisionSpace.getMarcherInstruction(col.firstMarcher);
                    MovingMarcher marcherMove2 = collisionSpace.getMarcherInstruction(col.secondMarcher);
                    
                    bool badSolutionDuration = false;
                    bool badSolutionDestination = false;
                    
                    SolverCoord destination1 = mutableMarcherSolution1.endPos;
                    SolverCoord destination2 = mutableMarcherSolution2.endPos;
                    if (newOption.swapMarchers != activeOption.swapMarchers)
                    {
                        SolverCoord tmp = destination1;
                        destination1 = destination2;
                        destination2 = tmp;
                    }
                    
                    recalculateMarcher(marcherMove1, mutableMarcherSolution1, instructionOptions[newOption.instructionForMarcher1], destination1);
                    recalculateMarcher(marcherMove2, mutableMarcherSolution2, instructionOptions[newOption.instructionForMarcher2], destination2);
                    badSolutionDuration = (marcherMove1.waitBeats + marcherMove1.numSteps.first + marcherMove1.numSteps.second > maxBeats) || (marcherMove2.waitBeats + marcherMove2.numSteps.first + marcherMove2.numSteps.second > maxBeats);
                    badSolutionDestination = !destinationConstraints.destinationIsAllowed(col.firstMarcher, mutableMarcherSolution1.endPos) || !destinationConstraints.destinationIsAllowed(col.secondMarcher, mutableMarcherSolution2.endPos);
                    
                    activeOption = newOption;
                    if ((badSolutionDuration || badSolutionDestination) && newOptionIndex != activeOptionIndex)
                    {
                        continue;
                    }
                    
                    collisionSpace.reinstructMarcher(col.firstMarcher, marcherMove1);
                    collisionSpace.reinstructMarcher(col.secondMarcher, marcherMove2);
                    
                    allActiveOptions[{col.firstMarcher, col.secondMarcher}] = newOptionIndex;
                    marcherSolutions[col.firstMarcher] = mutableMarcherSolution1;
                    marcherSolutions[col.secondMarcher] = mutableMarcherSolution2;
                    
                    if (executingFinalIteration)
                    {
                        break;
                    }
                    
                    // Track our improvement
                    if (collisionSpace.collectCollisionPairs().size() <= bestNumCollisions)
                    {
                        bestNumCollisions = (unsigned)collisionSpace.collectCollisionPairs().size();
                        bestOptionIndex = newOptionIndex;
//                        bestMarcher1 = marcherMove1;
//                        bestMarcher2 = marcherMove2;
                    }
                }
            }
        }
    }
};
    
}

# pragma mark - Algorithm By: Sover, Eliceiri, Hershkovitz

// ======================================================
// ===-- ALGORITHM BY: SOVER, ELICEIRI, HERSHKOVITZ --===
// ======================================================

namespace e7SoverEliceiriHershkovitz {
    
    // Assign initially by munkres
    // Get collisions
    // Create a placement order
    // Place marcher with an initial direction, wait, and target
    // Check against other marchers with those instructions
    // If it collides, then try to add a wait
    // If that doesn't work, add to the 'did not finish' list
    
    // Then -- RESET EVERYTHING
    // Give priority to everything that did not finish, and do it all again

    
void iterateSolution(std::vector<MarcherSolution>& marcherSolutions, CollisionSpace& collisionSpace, unsigned maxBeats, const  DestinationConstraints &destinationConstraints, const std::vector<TransitionSolverParams::InstructionOption> &instructionOptions, TransitionSolverDelegate *delegate)
{
    // Prioritize commands in the order that we will be willing to give them to a marcher
    // We're much more willing to change direction than to increase the number of wait beats
    std::vector<TransitionSolverParams::InstructionOption> prioritizedInstructionOptions = instructionOptions;
    auto comparator = [](const TransitionSolverParams::InstructionOption &first, const TransitionSolverParams::InstructionOption &second) {
        if (first.waitBeats == second.waitBeats) {
            return first.movementPattern < second.movementPattern;
        } else {
            return first.waitBeats < second.waitBeats;
        }
    };
    std::sort(prioritizedInstructionOptions.begin(), prioritizedInstructionOptions.end(), comparator);
    
    // Create a mapping of all of the marchers to their current instructions
    std::vector<unsigned> activeInstructions;
    for (unsigned i = 0; i < marcherSolutions.size(); i++)
    {
        activeInstructions.push_back(0);
    }
    
    // Create an initial order for setting down our marchers
    std::vector<unsigned> marchOrder;
    for (unsigned i = 0; i < marcherSolutions.size(); i++)
    {
        marchOrder.push_back(i);
    }
    
    std::vector<unsigned> unplacedMarchers = marchOrder;
    const unsigned maxIterations = 500;
    for (unsigned numIterations = 0; numIterations < maxIterations || collisionSpace.isSolved(); numIterations++)
    {
        if (delegate)
        {
            delegate->OnSubtaskProgress(std::max((double)(marchOrder.size() - unplacedMarchers.size()) / (double)marchOrder.size(), (double)numIterations / (double)maxIterations));
            if (delegate->ShouldAbortCalculation())
            {
                break;
            }
        }
        
        // Reset all of the marchers -- we will place them back one-by-one
        for (unsigned i = 0; i < marcherSolutions.size(); i++)
        {
            collisionSpace.disableMarcher(i);
        }
        
        // Place marchers onto the field in the march order
        unplacedMarchers.clear();
        for (unsigned marcher : marchOrder)
        {
            // Re-enable the marcher
            collisionSpace.enableMarcher(marcher);
            
            // Reset the marcher instruction back to one with the lowest wait time
            for (unsigned i = 0; i < prioritizedInstructionOptions.size(); i++)
            {
                if (prioritizedInstructionOptions[activeInstructions[marcher]].movementPattern == prioritizedInstructionOptions[i].movementPattern)
                {
                    activeInstructions[marcher] = i;
                    break;
                }
            }
            
            for (; activeInstructions[marcher] < prioritizedInstructionOptions.size(); activeInstructions[marcher]++)
            {
                MovingMarcher newMoveInstructions;
                e7ChiuZamoraMalani::recalculateMarcher(newMoveInstructions, marcherSolutions[marcher], prioritizedInstructionOptions[activeInstructions[marcher]], marcherSolutions[marcher].endPos);
                
                if (collisionSpace.collectCollisionPairs().size() == 0 && collisionSpace.getMarchersWithIncompleteTransitions().size() == 0) {
                    break;
                }
                
            }
            
            if (activeInstructions[marcher] == prioritizedInstructionOptions.size()) {
                activeInstructions[marcher] = 0;
                unplacedMarchers.push_back(marcher);
                collisionSpace.disableMarcher(marcher);
            }
        }
        
        // Remove all of the marchers that need to be re-prioritized
        {
            auto unplacedIter = unplacedMarchers.begin();
            for (auto marchersIter = marchOrder.begin(); marchersIter != marchOrder.end() && unplacedIter != unplacedMarchers.end(); marchersIter++)
            {
                if (*marchersIter == *unplacedIter)
                {
                    marchOrder.erase(marchersIter--);
                    unplacedIter++;
                }
            }
        }
        
        // Shuffle the people who need priority of replacement
        std::random_shuffle(unplacedMarchers.begin(), unplacedMarchers.end());
        
        // Re-add the unplaced marchers with higher priority
        marchOrder.insert(marchOrder.begin(), unplacedMarchers.begin(), unplacedMarchers.end());
    }
};
    
    
}

# pragma mark - Final Solver

// ========================
// ===-- FINAL SOLVER --===
// ========================

std::vector<std::string> validateSheetForTransitionSolver(const CC_sheet& sheet)
{
    std::vector<std::string>            errors;
    
    // Verify that all points are in the grid, and that all of them can be converted to and from solver space
    for (unsigned i = 0; i < sheet.GetPoints().size(); i++)
    {
        CC_coord            showPosition;
        SolverCoord         solverPosition;
        
        showPosition = sheet.GetPoint(i).GetPos();
        solverPosition = SolverCoord::fromShowSpace(showPosition);
        
        if (solverPosition.x % 2 != 0 || solverPosition.y % 2 != 0)
        {
            errors.push_back("Marcher " + std::to_string(i) + " is not located on a 2-step grid.");
        }
        if (solverPosition.toShowSpace() != showPosition)
        {
            errors.push_back("Bad location for marcher " + std::to_string(i));
        }
        
    }
    
    return errors;
}

TransitionSolverResult runSolverWithExplicitBeatCap(const CC_sheet& sheet1, const CC_sheet& sheet2, TransitionSolverParams params, unsigned numBeats, TransitionSolverDelegate *delegate) {
    
    TransitionSolverResult      results;
    
    std::vector<SolverCoord> startPositions;
    std::vector<SolverCoord> endPositions;
    fillPositionsFromSheet(sheet1, startPositions);
    fillPositionsFromSheet(sheet2, endPositions);
    
    auto fieldWidth = SolverCoord::kFieldWidthInSteps;
    auto fieldHeight = SolverCoord::kFieldHeightInSteps;
    unsigned maxBeats = numBeats;
    
    // Decrease the resolution of the problem
    fieldWidth /= 2;
    fieldHeight /= 2;
    maxBeats /= 2;
    for (unsigned i = 0; i < startPositions.size(); i++)
    {
        startPositions[i] /= 2;
        endPositions[i] /= 2;
    }
    
    Matrix<double>              distances;
    DestinationConstraints      destinationConstraints(params.groups, endPositions);
    distances = makeHungarianDistanceMatrix(startPositions, endPositions, maxBeats);
    for (unsigned i = 0; i < startPositions.size(); i++)
    {
        for (unsigned k = 0; k < endPositions.size(); k++)
        {
            if (!destinationConstraints.destinationIsAllowed(i, k))
            {
                distances(i, k) = std::numeric_limits<double>::max();
            }
        }
    }
    
    Munkres<double> solver;
    solver.solve(distances);
    
    std::vector<unsigned> assignments;
    for (size_t i = 0; i < startPositions.size(); i++) {
        for (size_t j = 0; j < startPositions.size(); j++) {
            if (distances(i, j) == 0) {
                assignments.push_back((unsigned)j);
            }
        }
    }
    
    std::vector<MarcherSolution> marcherSolutions(assignments.size());
    
    CollisionSpace collisionSpace(fieldWidth, fieldHeight, startPositions, maxBeats);
    
    std::vector<TransitionSolverParams::InstructionOption> instructionOptions;
    for (unsigned i = 0; i < params.availableInstructions.size(); i++)
    {
        if (params.availableInstructionsMask[i])
        {
            instructionOptions.push_back(params.availableInstructions[i]);
            instructionOptions[i].waitBeats /= 2;
        }
    }
    
    for (size_t i = 0; i < assignments.size(); i++) {
        marcherSolutions[i].startPos = startPositions[i];
        setupMarcherSolutionPath(marcherSolutions[i], instructionOptions[0], endPositions[assignments[i]]);
        
        refreshMarcherPathInCollisionSpace(collisionSpace, (unsigned)i, marcherSolutions[i]);
    }
    
    if (delegate)
    {
        delegate->OnSubtaskProgress(0);
    }
    switch (params.algorithm)
    {
        case TransitionSolverParams::AlgorithmIdentifier::E7_ALGORITHM__CHIU_ZAMORA_MALANI:
            e7ChiuZamoraMalani::iterateSolution(marcherSolutions, collisionSpace, maxBeats, destinationConstraints, instructionOptions, delegate);
            break;
        case TransitionSolverParams::AlgorithmIdentifier::E7_ALGORITHM__NAMINIASL_RAMIREZ_ZHANG:
            e7NaminiaslRamirezZhang::iterateSolution(marcherSolutions, collisionSpace, maxBeats, destinationConstraints, instructionOptions, delegate);
            break;
        case TransitionSolverParams::AlgorithmIdentifier::E7_ALGORITHM__SOVER_ELICEIRI_HERSHKOVITZ:
            e7SoverEliceiriHershkovitz::iterateSolution(marcherSolutions, collisionSpace, maxBeats, destinationConstraints, instructionOptions, delegate);
            break;
        default:
            break;
    }
    if (delegate)
    {
        delegate->OnSubtaskProgress(1);
    }
    
    results.successfullySolved = collisionSpace.isSolved();
    results.numBeatsOfMovement = collisionSpace.lastBeatContainingMovement() * 2;
    
    for (size_t i = 0; i < assignments.size(); i++) {
        results.finalPositions.push_back(SolverCoord::toShowSpace(marcherSolutions[i].endPos * 2)); // Make sure to scale the solution back up
    }
    
    std::map<std::pair<PathInstruction, unsigned>, SYMBOL_TYPE> instructionToDotType;
    for (unsigned i = 0; i < instructionOptions.size(); i++)
    {
        SYMBOL_TYPE                                         dotType = (SYMBOL_TYPE)i;
        std::string                                         instructionString;
        const TransitionSolverParams::InstructionOption     &instruction = instructionOptions.at(i);;
        
        switch (instruction.movementPattern)
        {
            case TransitionSolverParams::InstructionOption::Pattern::EWNS:
                instructionToDotType[std::make_pair(PATH_EWNS, instruction.waitBeats)] = dotType;
                instructionToDotType[std::make_pair(PATH_DEGENERATE_STRAIGHT, instruction.waitBeats)] = dotType;
                instructionString = "ewns";
                break;
            case TransitionSolverParams::InstructionOption::Pattern::NSEW:
                instructionToDotType[std::make_pair(PATH_NSEW, instruction.waitBeats)] = dotType;
                instructionToDotType[std::make_pair(PATH_DEGENERATE_STRAIGHT, instruction.waitBeats)] = dotType;
                instructionString = "nsew";
                break;
            case TransitionSolverParams::InstructionOption::Pattern::DMHS:
                instructionToDotType[std::make_pair(PATH_DMHS, instruction.waitBeats)] = dotType;
                instructionToDotType[std::make_pair(PATH_DEGENERATE_DIAGONAL, instruction.waitBeats)] = dotType;
                instructionString = "dmhs";
                break;
            case TransitionSolverParams::InstructionOption::Pattern::HSDM:
                instructionToDotType[std::make_pair(PATH_HSDM, instruction.waitBeats)] = dotType;
                instructionToDotType[std::make_pair(PATH_DEGENERATE_DIAGONAL, instruction.waitBeats)] = dotType;
                instructionString = "hsdm";
                break;
            default:
                break;
        }
        
        results.continuities[dotType] = "mt " + std::to_string(instruction.waitBeats * 2) + " e" + "\n" + instructionString + " np";
    }
    
    for (unsigned i = 0; i < marcherSolutions.size(); i++)
    {
        const MarcherSolution &solution = marcherSolutions[i];
        
        results.marcherDotTypes.push_back(instructionToDotType[std::make_pair(solution.instruction, solution.waitBeats)]);
    }
    
    return results;
}

TransitionSolverResult runTransitionSolver(const CC_sheet& sheet1, const CC_sheet& sheet2, TransitionSolverParams params, TransitionSolverDelegate *delegate) {
    
    TransitionSolverResult              finalResult;
    TransitionSolverResult              recentResult;
    unsigned                            scaledHighestBeatCap;
    unsigned                            scaledBeatCapForBestSolution;
    unsigned                            scaledBeatCapForCurrentCalculation;
    unsigned                            numBeatsOfMovementInBestSolution;
    
    scaledBeatCapForBestSolution = 0;
    scaledHighestBeatCap = (sheet1.GetBeats() + (sheet1.GetBeats() / 2)) / 2;
    scaledBeatCapForCurrentCalculation = 0;
    numBeatsOfMovementInBestSolution = sheet1.GetBeats();
    finalResult.successfullySolved = false;
    
    if (params.algorithm == TransitionSolverParams::AlgorithmIdentifier::E7_ALGORITHM__NAMINIASL_RAMIREZ_ZHANG)
    {
        scaledHighestBeatCap = (sheet1.GetBeats() / 2);
    }
    
    while (scaledBeatCapForCurrentCalculation <= scaledHighestBeatCap) {
        if (delegate) {
            delegate->OnProgress(((double)scaledBeatCapForCurrentCalculation) / (double)scaledHighestBeatCap);
            if (delegate->ShouldAbortCalculation())
            {
                break;
            }
        }
        
        recentResult = runSolverWithExplicitBeatCap(sheet1, sheet2, params, scaledBeatCapForCurrentCalculation * 2, delegate);
        
        if (recentResult.successfullySolved) {
            printf("%d, %d\n", scaledBeatCapForCurrentCalculation, recentResult.numBeatsOfMovement);
            
            if (recentResult.numBeatsOfMovement <= numBeatsOfMovementInBestSolution)
            {
                numBeatsOfMovementInBestSolution = recentResult.numBeatsOfMovement;
                scaledBeatCapForBestSolution = scaledBeatCapForCurrentCalculation;
                finalResult = recentResult;
                if (delegate)
                {
                    delegate->OnNewPreferredSolution(numBeatsOfMovementInBestSolution);
                }
            }

        }
        
        scaledBeatCapForCurrentCalculation += 1;
    }
    

// BINARY SEARCH VERSION -- turns out to be less-good
//    TransitionSolverResult              recentResult;
//    std::pair<unsigned, unsigned>       scaledBeatLimits;
//    unsigned                            scaledBeatCapForBestSolution;
//    unsigned                            scaledBeatCapForPreviousCalculation;
//    unsigned                            scaledBeatCapForCurrentCalculation;
//    unsigned                            numBeatsOfMovementInBestSolution;
//    
//    scaledBeatLimits.first = 0;
//    scaledBeatLimits.second = sheet1.GetBeats() / 2;
//    scaledBeatLimits.second += scaledBeatLimits.second / 2;
//    scaledBeatCapForBestSolution = 0;
//    scaledBeatCapForCurrentCalculation = scaledBeatLimits.second;
//    scaledBeatCapForPreviousCalculation = 0;
//    numBeatsOfMovementInBestSolution = sheet1.GetBeats();
//
//    while (scaledBeatCapForCurrentCalculation != scaledBeatCapForPreviousCalculation) {
//        recentResult = runSolverWithExplicitBeatCap(sheet1, sheet2, params, scaledBeatCapForCurrentCalculation * 2);
//        
//        if (recentResult.successfullySolved) {
//            scaledBeatLimits.second = scaledBeatCapForCurrentCalculation;
//            
//            if (recentResult.numBeatsOfMovement <= numBeatsOfMovementInBestSolution)
//            {
//                numBeatsOfMovementInBestSolution = recentResult.numBeatsOfMovement;
//                scaledBeatCapForBestSolution = scaledBeatCapForCurrentCalculation;
//            }
//            
//        } else {
//                scaledBeatLimits.first = scaledBeatCapForCurrentCalculation;
//        }
//
//        scaledBeatCapForPreviousCalculation = scaledBeatCapForCurrentCalculation;
//        scaledBeatCapForCurrentCalculation = scaledBeatLimits.first + ((scaledBeatLimits.second - scaledBeatLimits.first) / 2);
//    }
    
    if (delegate)
    {
        delegate->OnCalculationComplete(finalResult);
    }
    return finalResult;
}

