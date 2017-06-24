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
    
    // Cached states
    mutable bool m_cachedCollisionsNeedRefresh;
    mutable unsigned m_cachedCollisionsBeat;
    mutable std::vector<Collision> m_cachedCollisionPairs;
    mutable bool m_clipBeatCollisionsNeedRefresh;
    mutable std::vector<Collision> m_cachedCollisionsForClipBeat;
    
public:
    bool isSolved() const;
    
    void reinstructMarcher(unsigned which, const MovingMarcher& newMarcherAnimation);
    void clipToBeat(unsigned clipBeat);
    
    std::set<unsigned> getMarchersWithIncompleteTransitions() const;
    
    std::vector<Collision> collectCollisionPairs() const;
    std::vector<Collision> collectCollisionPairs(unsigned maxBeat) const;
    unsigned beatsBeforeCollisionBetweenMarchers(unsigned marcher1, unsigned marcher2) const;
    
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
    return (m_clipBeat == m_numBeats) && (collectCollisionPairs().size() == 0) && (getMarchersWithIncompleteTransitions().size() == 0);
}

std::set<unsigned> CollisionSpace::getMarchersWithIncompleteTransitions() const {
    return m_incompleteTransitions;
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
                collisions.push_back({firstColBeat, i, j, m_collisions.at(i).at(j).at(firstColBeat).toShowSpace(m_marcherGrid.size(), m_marcherGrid[0].size())});
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
    instructMarcher(which, newMarcherAnimation);
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
    for (auto i = 0; i < points.size(); i++) {
        positions.push_back(SolverCoord::fromShowSpace(points[i].GetPos()));
    }
}

Matrix<double> makeHungarianDistanceMatrix(const std::vector<SolverCoord>& startPositions, const std::vector<SolverCoord>& endPositions, unsigned numBeats) {
    size_t numMarchers = startPositions.size();
    Matrix<double> matrix = Matrix<double>(numMarchers, numMarchers);
    for (auto i = 0; i < endPositions.size(); i++) {
        for (auto j = 0; j < startPositions.size(); j++) {
            auto diff = startPositions.at(j) - endPositions.at(i);
            auto manhattanDist = abs(diff.x) + abs(diff.y);
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
        case PATH_DMHS:
            majorStepIsFirst = true;
        case PATH_HSDM:
            majorStep += minorStep;
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

void setMarcherDestination(MarcherSolution& marcher, SolverCoord newEndPos, PathAxes& marcherPathAxes)
{
    marcher.endPos = newEndPos;
    marcherPathAxes =  calcPathAxes(marcher.startPos, marcher.endPos);
}

void refreshMarcherPathInCollisionSpace(CollisionSpace& colSpace, unsigned marcherIndex, const MarcherSolution& marcherSol, const PathAxes& cachedPathAxes)
{
    std::pair<SolverCoord, SolverCoord> stepVectors = calcStepVectors(cachedPathAxes, marcherSol.instruction);
    std::pair<float, float> numSteps = calcStepsInEachDir(marcherSol.startPos, marcherSol.endPos, stepVectors.first, stepVectors.second);
    
    MovingMarcher marcherAnim;
    marcherAnim.waitBeats = marcherSol.waitBeats;
    marcherAnim.stepVectors = stepVectors;
    marcherAnim.numSteps = numSteps;
    marcherAnim.startPos = marcherSol.startPos;
    
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
                    
                    if (gridOccupant > maxMarcherIndex)
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
typedef std::tuple<bool, bool, unsigned, unsigned, bool> SolutionAdjustmentInstruction;

bool adjustmentInstructionSorter (SolutionAdjustmentInstruction first,SolutionAdjustmentInstruction second) {
    return (std::get<2>(first) + std::get<3>(first))< (std::get<2>(second) + std::get<3>(second));
}
    
std::vector<SolutionAdjustmentInstruction> unfilteredAdjustmentOptions(unsigned numBeats)
{
    std::vector<SolutionAdjustmentInstruction> allOptions;
    for (bool swapDir1 : {false, true}) {
        for (bool swapDir2 : {false, true}) {
            for (unsigned waitBeats1 = 0; waitBeats1 <= numBeats; waitBeats1++) {
                for (unsigned waitBeats2 = 0; waitBeats2 <= numBeats; waitBeats2++) {
                    for (bool swapPositions : {false, true}) {
                        allOptions.push_back({swapDir1, swapDir2, waitBeats1, waitBeats2, swapPositions});
                    }
                }
            }
        }
    }
    std::sort(allOptions.begin(), allOptions.end(), adjustmentInstructionSorter);
    return allOptions;
}
    
std::vector<SolutionAdjustmentInstruction> filterAdjustmentOptionsWithPathSwap(const std::vector<SolutionAdjustmentInstruction>& unfilteredOptions, bool firstMarcher)
{
    std::vector<SolutionAdjustmentInstruction> filteredOptions(unfilteredOptions);
    for (unsigned indexPlusOne = unfilteredOptions.size(); indexPlusOne > 0; indexPlusOne--) {
        auto& option = unfilteredOptions.at(indexPlusOne - 1);
        bool remove = false;
        if (firstMarcher) {
            remove = std::get<0>(option);
        } else {
            remove = std::get<1>(option);
        }
        if (remove) {
            filteredOptions.erase(filteredOptions.begin() + indexPlusOne - 1);
        }
    }
    return filteredOptions;
}

bool attemptAdjustWaitTime(MovingMarcher& marcherToAdjust,MarcherSolution& marcherSolution, unsigned waitBeats, unsigned maxBeats) {
    if (marcherToAdjust.numSteps.first + marcherToAdjust.numSteps.second + waitBeats > maxBeats) {
        return false;
    }
    marcherToAdjust.waitBeats = marcherSolution.waitBeats = waitBeats;
    return true;
}
    
void swapMoveDirections(MovingMarcher& marcherToAdjust, MarcherSolution& marcherSolution) {
    if (!(marcherToAdjust.stepVectors.second == 0))
    {
        SolverCoord tempStepVector = marcherToAdjust.stepVectors.second;
        marcherToAdjust.stepVectors.second = marcherToAdjust.stepVectors.first;
        marcherToAdjust.stepVectors.first = tempStepVector;
        
        unsigned tempNumSteps = marcherToAdjust.numSteps.second;
        marcherToAdjust.numSteps.second = marcherToAdjust.numSteps.first;
        marcherToAdjust.numSteps.first = tempNumSteps;
    }
    
    if (marcherSolution.instruction == PATH_NSEW) {
        marcherSolution.instruction = PATH_EWNS;
    } else if (marcherSolution.instruction == PATH_EWNS) {
        marcherSolution.instruction = PATH_NSEW;
    }
}

bool attemptSetDestination(MovingMarcher& marcher, MarcherSolution& marcherSolution, SolverCoord newDestination, unsigned maxBeats) {
    PathAxes newPathAxes = calcPathAxes(marcher.startPos, newDestination);
    marcherSolution.instruction = PATH_EWNS;
    marcher.stepVectors = calcStepVectors(newPathAxes, PATH_EWNS);
    marcher.numSteps = calcStepsInEachDir(marcher.startPos, newDestination, marcher.stepVectors.first, marcher.stepVectors.second);
    marcher.waitBeats = marcherSolution.waitBeats = 0;
    marcherSolution.endPos = newDestination;
    return (marcher.numSteps.first + marcher.numSteps.second + marcher.waitBeats <= maxBeats);
}


void iterateSolution(std::vector<MarcherSolution>& marcherSolutions, CollisionSpace& collisionSpace, unsigned maxBeats)
{
//    const std::vector<SolutionAdjustmentInstruction> unfilteredOptions = unfilteredAdjustmentOptions(maxBeats);
    const std::vector<SolutionAdjustmentInstruction> unfilteredOptions = unfilteredAdjustmentOptions(3);
    const std::vector<SolutionAdjustmentInstruction> optionsForDegenerateFirstPath = filterAdjustmentOptionsWithPathSwap(unfilteredOptions, true);
    const std::vector<SolutionAdjustmentInstruction> optionsForDegenerateSecondPath = filterAdjustmentOptionsWithPathSwap(unfilteredOptions, false);
    const std::vector<SolutionAdjustmentInstruction> optionsForTwoDegeneratePaths = filterAdjustmentOptionsWithPathSwap(optionsForDegenerateFirstPath, false);
    
    std::vector<Collision> collisionPairs;
    std::vector<bool> degeneratePaths;
    std::map<std::pair<unsigned, unsigned>, unsigned> allActiveOptions;
    
    for (unsigned i = 0; i < marcherSolutions.size(); i++) {
        SolverCoord marcherMoveDiff = marcherSolutions[i].endPos - marcherSolutions[i].startPos;
        degeneratePaths.push_back(marcherMoveDiff.x == 0 || marcherMoveDiff.y == 0);
    }
    
    bool progress = true;
    unsigned leastNumCollisions = collisionSpace.collectCollisionPairs().size();
    unsigned numIterationsWithoutOverallImprovement = 0;
    while (!collisionSpace.isSolved() && progress && numIterationsWithoutOverallImprovement < 10) {
        
        progress = false;
        
        for (unsigned beat = 0; beat <= maxBeats; beat++) {
            
            collisionSpace.clipToBeat(beat);
            collisionPairs = collisionSpace.collectCollisionPairs(beat);
            
            for (unsigned collisionIndex = 0; collisionIndex < collisionPairs.size(); collisionIndex++) {
                Collision& col = collisionPairs[collisionIndex];
                
                // For each pair of colliding marchers, select the appropriate set of options for solving the collision
                const std::vector<SolutionAdjustmentInstruction>* adjustmentOptionsPtr;
//                if (degeneratePaths[col.firstMarcher] && degeneratePaths[col.secondMarcher]) {
//                    adjustmentOptionsPtr = &optionsForTwoDegeneratePaths;
//                } else if (degeneratePaths[col.firstMarcher]) {
//                    adjustmentOptionsPtr = &optionsForDegenerateFirstPath;
//                } else if (degeneratePaths[col.secondMarcher]) {
//                    adjustmentOptionsPtr = &optionsForDegenerateSecondPath;
//                } else {
                    adjustmentOptionsPtr = &unfilteredOptions;
//                }
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
                    
                    // Swap marchers
                    if (std::get<4>(newOption) != std::get<4>(activeOption)) { // If the new swap setting is different from the old one, then switch
                        SolverCoord dest1 = mutableMarcherSolution1.endPos;
                        SolverCoord dest2 = mutableMarcherSolution2.endPos;
                        badSolutionDuration = !attemptSetDestination(marcherMove1, mutableMarcherSolution1, dest2, maxBeats) | !attemptSetDestination(marcherMove2, mutableMarcherSolution2, dest1, maxBeats);
                        // TODO: ??? Recalculate whether or not we're degenerate; recalculate the active option
                    }
                    
                    // Wait time, marcher 1
                    if (std::get<2>(newOption) != std::get<2>(activeOption)) { // If the new option has a different wait time, apply it
                        badSolutionDuration = badSolutionDuration | !attemptAdjustWaitTime(marcherMove1, mutableMarcherSolution1, std::get<2>(newOption), maxBeats);
                    }
                    
                    // Wait time, marcher 2
                    if (std::get<3>(newOption) != std::get<3>(activeOption)) { // If the new option has a different wait time, apply it
                        badSolutionDuration = badSolutionDuration | !attemptAdjustWaitTime(marcherMove2, mutableMarcherSolution2, std::get<3>(newOption), maxBeats);
                    }
                    
                    // Swap directions, marcher 1
                    if (std::get<0>(newOption) != std::get<0>(activeOption)) { // If the new option involves a new move direction, apply it
                        swapMoveDirections(marcherMove1, mutableMarcherSolution1);
                    }
                    
                    // Swap directions, marcher 2
                    if (std::get<1>(newOption) != std::get<1>(activeOption)) { // If the new option involves a new move direction, apply it
                        swapMoveDirections(marcherMove2, mutableMarcherSolution2);
                    }
                    
                    activeOption = newOption;
                    if (badSolutionDuration && newOptionIndex != activeOptionIndex)
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
        
        unsigned numCollisions = collisionSpace.collectCollisionPairs().size();
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
    
    
}

# pragma mark - Final Solver

// ========================
// ===-- FINAL SOLVER --===
// ========================

std::tuple<bool, std::vector<CC_coord>, std::vector<CC_coord>, std::vector<std::string>, std::vector<SYMBOL_TYPE>> runSolver(const CC_sheet& sheet1, const CC_sheet& sheet2, unsigned numBeats) {
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
    
    Matrix<double> distances;
    distances = makeHungarianDistanceMatrix(startPositions, endPositions, maxBeats);
    
    Munkres<double> solver;
    solver.solve(distances);
    
    std::vector<unsigned> assignments;
    for (auto i = 0; i < startPositions.size(); i++) {
        for (auto j = 0; j < startPositions.size(); j++) {
            if (distances(i, j) == 0) {
                assignments.push_back(j);
            }
        }
    }
    
    std::vector<MarcherSolution> marcherSolutions(assignments.size());
    std::vector<PathAxes> marcherMoveDirections(assignments.size());
    
    CollisionSpace collisionSpace(fieldWidth, fieldHeight, startPositions, maxBeats);
    
    for (auto i = 0; i < assignments.size(); i++) {
        marcherSolutions[i].startPos = startPositions[i];
        marcherSolutions[i].instruction = PATH_NSEW;
        
        setMarcherDestination(marcherSolutions[i], endPositions[assignments[i]], marcherMoveDirections[i]);
        refreshMarcherPathInCollisionSpace(collisionSpace, i, marcherSolutions[i], marcherMoveDirections[i]);
    }
    
    e7ChiuZamoraMalani::iterateSolution(marcherSolutions, collisionSpace, maxBeats);
    
    std::vector<CC_coord> finalPositions;
    std::vector<CC_coord> finalStartPositions;
    for (auto i = 0; i < assignments.size(); i++) {
        finalPositions.push_back(SolverCoord::toShowSpace(marcherSolutions[i].endPos * 2)); // Make sure to scale the solution back up
        finalStartPositions.push_back(SolverCoord::toShowSpace(marcherSolutions[i].startPos * 2)); // Make sure to scale the solution back up
    }
    
    std::vector<std::string> finalContinuities;
    for (SYMBOL_TYPE symbol = SYMBOLS_START; symbol < MAX_NUM_SYMBOLS; symbol = (SYMBOL_TYPE)(((unsigned)symbol) + 1))
    {
        PathInstruction     dotTypeInstruction;
        unsigned            dotTypeWait;
        
        dotTypeInstruction = (((unsigned)symbol) / 4) == 0 ? PATH_EWNS : PATH_NSEW;
        dotTypeWait = (((unsigned)symbol) % 4) * 2;
        
        finalContinuities.push_back("mt " + std::to_string(dotTypeWait) + " e" + "\n" + (dotTypeInstruction == PATH_EWNS ? "ewns np" : "nsew np"));
    }
    
    std::vector<SYMBOL_TYPE> finalDotTypes;
    for (unsigned i = 0; i < marcherSolutions.size(); i++)
    {
        const MarcherSolution &solution = marcherSolutions[i];
        PathInstruction effectiveInstruction = solution.instruction;
        if (effectiveInstruction == PATH_DEGENERATE_STRAIGHT)
        {
            effectiveInstruction = PATH_EWNS;
        }
        
        finalDotTypes.push_back((SYMBOL_TYPE)((solution.instruction == PATH_EWNS ? 0 : 1) * 4 + solution.waitBeats));
    }
    
    return std::make_tuple(collisionSpace.isSolved(), finalStartPositions, finalPositions, finalContinuities, finalDotTypes);
    
}

