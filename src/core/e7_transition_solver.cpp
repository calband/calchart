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
    
    bool operator==(const SolverCoord& other) {
        return (this->x == other.x && this->y == other.y);
    }
    bool operator==(int32_t scalar) {
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
};

struct NotEnoughTimeException {};


class CollisionSpace {
public:
    CollisionSpace(unsigned gridXSize, unsigned gridYSize, const std::vector<SolverCoord>& marcherStartPositions, unsigned maxBeats);
//private:
    unsigned m_numBeats;
    std::vector<std::vector<std::vector<std::set<unsigned>>>> m_marcherGrid;
    std::vector<std::vector<std::map<unsigned, SolverCoord>>> m_collisions;
    std::vector<MovingMarcher> m_marchers;
    
    std::vector<Collision> collectCollisionPairs() const;
    std::vector<Collision> collectCollisionPairs(unsigned maxBeat) const;
    unsigned beatsBeforeCollisionBetweenMarchers(unsigned marcher1, unsigned marcher2) const;
    
    const MovingMarcher& getMarcherInstruction(unsigned marcherIndex) const;
    
    void rollbackMarcherInstructions(unsigned which);
    void instructMarcher(unsigned which, const MovingMarcher& newInstructions);
    void reinstructMarcher(unsigned which, const MovingMarcher& newMarcherAnimation);
    
    const std::set<unsigned>& getMarchersAt(unsigned gridX, unsigned gridY, unsigned beat) const;
    std::set<unsigned>& getMarchersAt(unsigned gridX, unsigned gridY, unsigned beat);
    
    void _forgetCollision(unsigned firstMarcher, unsigned secondMarcher, unsigned beat);
    void _registerCollision(unsigned firstMarcher, unsigned secondMarcher, unsigned beat, SolverCoord pos);
    
    void forgetCollision(unsigned firstMarcher, unsigned secondMarcher, unsigned beat);
    void registerCollision(unsigned firstMarcher, unsigned secondMarcher, unsigned beat, SolverCoord pos);
    
    void removeMarcher(unsigned marcher, unsigned gridX, unsigned gridY, unsigned beat);
    void placeMarcher(unsigned marcher, unsigned gridX, unsigned gridY, unsigned beat);
};


CollisionSpace::CollisionSpace(unsigned gridXSize, unsigned gridYSize, const std::vector<SolverCoord>& marcherStartPositions, unsigned maxBeats)
: m_numBeats(maxBeats)
{
    m_marcherGrid.resize(gridXSize);
    for (auto yGridIter = m_marcherGrid.begin(); yGridIter != m_marcherGrid.end(); yGridIter++) {
        auto& yGrid = *yGridIter;
        yGrid.resize(gridYSize);
        for (auto beatsVecIter = yGrid.begin(); beatsVecIter != yGrid.end(); beatsVecIter++) {
            auto& beatsVec = *beatsVecIter;
            beatsVec.resize(maxBeats);
        }
    }
    m_collisions.resize(marcherStartPositions.size());
    for (auto collisionsIter = m_collisions.begin(); collisionsIter != m_collisions.end(); collisionsIter++) {
        (*collisionsIter).resize(marcherStartPositions.size());
    }
    m_marchers.resize(marcherStartPositions.size());
    for (unsigned marcherIndex = 0; marcherIndex < marcherStartPositions.size(); marcherIndex++) {
        MovingMarcher marcher = {
            .waitBeats = 0,
            .stepVectors = { SolverCoord(), SolverCoord() },
            .numSteps = { 0, 0 },
            .startPos = marcherStartPositions.at(marcherIndex)
        };
        instructMarcher(marcherIndex, marcher);
    }
}

std::vector<Collision> CollisionSpace::collectCollisionPairs() const {
    return collectCollisionPairs(m_numBeats);
}

std::vector<Collision> CollisionSpace::collectCollisionPairs(unsigned maxBeat) const {
    std::vector<Collision> collisions;
    for (unsigned i = 0; i < m_marchers.size(); i++) {
        for (unsigned j = i + 1; j < m_marchers.size(); j++) {
            unsigned firstColBeat = beatsBeforeCollisionBetweenMarchers(i, j);
            if (firstColBeat < maxBeat) {
                collisions.push_back({firstColBeat, i, j});
            }
        }
    }
    return collisions;
}

unsigned CollisionSpace::beatsBeforeCollisionBetweenMarchers(unsigned marcher1, unsigned marcher2) const {
    auto& collisionsBetweenMarchers = m_collisions[marcher1][marcher2];
    unsigned firstColBeat = m_numBeats;
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

void CollisionSpace::rollbackMarcherInstructions(unsigned which) {
    auto& marcher = m_marchers[which];
    SolverCoord pos = marcher.startPos;
    unsigned beat = 0;
    
    unsigned waitPhaseEnd = marcher.waitBeats;
    std::pair<unsigned, unsigned> movePhaseEnds;
    movePhaseEnds.first = waitPhaseEnd + marcher.numSteps.first;
    movePhaseEnds.second = movePhaseEnds.first + marcher.numSteps.second;
    
    for (; beat < waitPhaseEnd; beat++) {
        removeMarcher(which, pos.x, pos.y, beat);
    }
    for (; beat < movePhaseEnds.first; beat++) {
        pos += marcher.stepVectors.first;
        removeMarcher(which, pos.x, pos.y, beat);
    }
    for (; beat < movePhaseEnds.second; beat++) {
        pos += marcher.stepVectors.second;
        removeMarcher(which, pos.x, pos.y, beat);
    }
    for (; beat < m_numBeats; beat++) {
        removeMarcher(which, pos.x, pos.y, beat);
    }
}

void CollisionSpace::instructMarcher(unsigned which, const MovingMarcher& newInstructions) {
    m_marchers[which] = newInstructions;
    auto& marcher = m_marchers[which];
    SolverCoord pos = marcher.startPos;
    unsigned beat = 0;
    
    unsigned waitPhaseEnd = marcher.waitBeats;
    std::pair<unsigned, unsigned> movePhaseEnds;
    movePhaseEnds.first = waitPhaseEnd + marcher.numSteps.first;
    movePhaseEnds.second = movePhaseEnds.first + marcher.numSteps.second;
    
    if (movePhaseEnds.second > m_numBeats) {
        throw NotEnoughTimeException();
    }
    
    for (; beat < waitPhaseEnd; beat++) {
        placeMarcher(which, pos.x, pos.y, beat);
    }
    for (; beat < movePhaseEnds.first; beat++) {
        pos += marcher.stepVectors.first;
        placeMarcher(which, pos.x, pos.y, beat);
    }
    for (; beat < movePhaseEnds.second; beat++) {
        pos += marcher.stepVectors.second;
        placeMarcher(which, pos.x, pos.y, beat);
    }
    for (; beat < m_numBeats; beat++) {
        placeMarcher(which, pos.x, pos.y, beat);
    }
}

void CollisionSpace::reinstructMarcher(unsigned which, const MovingMarcher& newMarcherAnimation) {
    rollbackMarcherInstructions(which);
    instructMarcher(which, newMarcherAnimation);
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

void CollisionSpace::removeMarcher(unsigned marcher, unsigned gridX, unsigned gridY, unsigned beat) {
    auto& marchersAtGridLoc = getMarchersAt(gridX, gridY, beat);
    marchersAtGridLoc.erase(marcher);
    for (auto otherMarcher : marchersAtGridLoc) {
        forgetCollision(marcher, otherMarcher, beat);
    }
}

void CollisionSpace::placeMarcher(unsigned marcher, unsigned gridX, unsigned gridY, unsigned beat) {
    auto& marchersAtGridLoc = getMarchersAt(gridX, gridY, beat);
    for (auto otherMarcher : marchersAtGridLoc) {
        registerCollision(marcher, otherMarcher, beat, CC_coord(gridX, gridY));
        // TODO: What about collisions from swapping places?
    }
    marchersAtGridLoc.insert(marcher);
}

void fillPositionsFromSheet(const CC_sheet& sheet, std::vector<SolverCoord>& positions) {
    auto points = sheet.GetPoints();
    for (auto i = 0; i < points.size(); i++) {
        positions.push_back(SolverCoord::fromShowSpace(points[i].GetPos()));
    }
}

Matrix<float> makeHungarianDistanceMatrix(const std::vector<SolverCoord>& startPositions, const std::vector<SolverCoord>& endPositions, unsigned numBeats) {
    size_t numMarchers = startPositions.size();
    Matrix<float> matrix = Matrix<float>(numMarchers, numMarchers);
    for (auto i = 0; i < startPositions.size(); i++) {
        for (auto j = 0; j < endPositions.size(); j++) {
            auto diff = startPositions.at(i) - endPositions.at(j);
            auto manhattanDist = abs(diff.x) + abs(diff.y);
            if (manhattanDist > numBeats) {
                matrix(i, j) = std::numeric_limits<float>::infinity();
            } else {
                matrix(i, j) = manhattanDist;
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
            majorStepIsFirst = (majorStep.y != 0);
            break;
        case PATH_NSEW:
            majorStepIsFirst = (majorStep.x != 0);
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
    float numSteps2 = (moveVector.y - (a * vec1.y)) / (vec2.y - (b * vec1.y));
    
    float numSteps1 = a - (b * numSteps2);

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
            for (unsigned waitBeats1 = 0; waitBeats1 < numBeats; waitBeats1 += 2) {
                for (unsigned waitBeats2 = 0; waitBeats2 < numBeats; waitBeats2 += 2) {
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

bool attemptAdjustWaitTime(MovingMarcher& marcherToAdjust, unsigned waitBeats, unsigned maxBeats) {
    if (marcherToAdjust.numSteps.first + marcherToAdjust.numSteps.second + waitBeats > maxBeats) {
        return false;
    }
    marcherToAdjust.waitBeats = waitBeats;
    return true;
}
    
void swapMoveDirections(MovingMarcher& marcherToAdjust, MarcherSolution& marcherSolution) {
    SolverCoord tempStepVector = marcherToAdjust.stepVectors.second;
    marcherToAdjust.stepVectors.second = marcherToAdjust.stepVectors.first;
    marcherToAdjust.stepVectors.first = tempStepVector;
    
    unsigned tempNumSteps = marcherToAdjust.numSteps.second;
    marcherToAdjust.numSteps.second = marcherToAdjust.numSteps.first;
    marcherToAdjust.numSteps.first = tempNumSteps;
    
    if (marcherSolution.instruction == PATH_NSEW) {
        marcherSolution.instruction = PATH_EWNS;
    } else {
        marcherSolution.instruction = PATH_NSEW;
    }
}

bool attemptSetDestination(MovingMarcher& marcher, SolverCoord newDestination, unsigned maxBeats) {
    PathAxes newPathAxes = calcPathAxes(marcher.startPos, newDestination);
    marcher.stepVectors = calcStepVectors(newPathAxes, PATH_EWNS);
    marcher.numSteps = calcStepsInEachDir(marcher.startPos, newDestination, marcher.stepVectors.first, marcher.stepVectors.second);
    marcher.waitBeats = 0;
    return (marcher.numSteps.first + marcher.numSteps.second + marcher.waitBeats <= maxBeats);
}


void iterateSolution(std::vector<MarcherSolution>& marcherSolutions, CollisionSpace& collisionSpace, unsigned maxBeats)
{
    const std::vector<SolutionAdjustmentInstruction> unfilteredOptions = unfilteredAdjustmentOptions(maxBeats);
    const std::vector<SolutionAdjustmentInstruction> optionsForDegenerateFirstPath = filterAdjustmentOptionsWithPathSwap(unfilteredOptions, true);
    const std::vector<SolutionAdjustmentInstruction> optionsForDegenerateSecondPath = filterAdjustmentOptionsWithPathSwap(unfilteredOptions, false);
    const std::vector<SolutionAdjustmentInstruction> optionsForTwoDegeneratePaths = filterAdjustmentOptionsWithPathSwap(optionsForDegenerateFirstPath, false);
    
    std::vector<Collision> collisionPairs = collisionSpace.collectCollisionPairs();
    std::vector<bool> degeneratePaths;
    std::map<std::pair<unsigned, unsigned>, unsigned> allActiveOptions;
    
    for (unsigned i = 0; i < marcherSolutions.size(); i++) {
        SolverCoord marcherMoveDiff = marcherSolutions[i].endPos - marcherSolutions[i].startPos;
        degeneratePaths.push_back(marcherMoveDiff.x == 0 || marcherMoveDiff.y == 0);
    }
    
    while (collisionPairs.size() > 0) {
        for (unsigned beat = 0; beat < maxBeats; beat++) {
            for (unsigned collisionIndex = 0; collisionIndex < collisionPairs.size(); collisionIndex++) {
                // Ignore collisions that happen outside the beats we're concerned with
                Collision& col = collisionPairs[collisionIndex];
                if (col.beat > beat) {
                    continue;
                }
                
                // For each pair of colliding marchers, select the appropriate set of options for solving the collision
                const std::vector<SolutionAdjustmentInstruction>* adjustmentOptionsPtr;
                if (degeneratePaths[col.firstMarcher] && degeneratePaths[col.secondMarcher]) {
                    adjustmentOptionsPtr = &optionsForTwoDegeneratePaths;
                } else if (degeneratePaths[col.firstMarcher]) {
                    adjustmentOptionsPtr = &optionsForDegenerateFirstPath;
                } else if (degeneratePaths[col.secondMarcher]) {
                    adjustmentOptionsPtr = &optionsForDegenerateSecondPath;
                } else {
                    adjustmentOptionsPtr = &unfilteredOptions;
                }
                const std::vector<SolutionAdjustmentInstruction>& adjustmentOptions = *adjustmentOptionsPtr;
                
                // Find the most recent option that was used to fix the collision; start from there when figuring out what to do next with them
                unsigned activeOptionIndex = allActiveOptions[{col.firstMarcher, col.secondMarcher}];
                SolutionAdjustmentInstruction activeOption = adjustmentOptions.at(activeOptionIndex);
                
                // Keep working on the collision until you find a solution
                for (unsigned newOptionIndex = activeOptionIndex; newOptionIndex < adjustmentOptions.size(); newOptionIndex++) {
                    SolutionAdjustmentInstruction newOption = adjustmentOptions.at(newOptionIndex);
                    
                    MovingMarcher marcherMove1 = collisionSpace.getMarcherInstruction(col.firstMarcher);
                    MovingMarcher marcherMove2 = collisionSpace.getMarcherInstruction(col.secondMarcher);
                    
                    bool appliedSwap = false;
                    
                    // Swap marchers
                    if (std::get<4>(newOption) != std::get<4>(activeOption)) {
                        if (!attemptSetDestination(marcherMove1, marcherSolutions[col.secondMarcher].endPos, maxBeats) || !attemptSetDestination(marcherMove2, marcherSolutions[col.firstMarcher].endPos, maxBeats)) {
                            continue;
                        }
                        appliedSwap = true;
                        // The swap function automatically sets the direction instruction to EWNS
                        std::get<0>(activeOption) = false;
                        std::get<1>(activeOption) = false;
                    }
                    
                    // Wait time, marcher 1
                    if (std::get<2>(newOption) != std::get<2>(activeOption)) {
                        if (!attemptAdjustWaitTime(marcherMove1, std::get<2>(newOption), maxBeats)) {
                            continue;
                        }
                    }
                    
                    // Wait time, marcher 2
                    if (std::get<3>(newOption) != std::get<3>(activeOption)) {
                        if (!attemptAdjustWaitTime(marcherMove2, std::get<3>(newOption), maxBeats)) {
                            continue;
                        }
                    }
                    
                    // Swap directions, marcher 1
                    if (std::get<0>(newOption) != std::get<0>(activeOption)) {
                        swapMoveDirections(marcherMove1, marcherSolutions[col.firstMarcher]);
                    }
                    
                    // Swap directions, marcher 2
                    if (std::get<1>(newOption) != std::get<1>(activeOption)) {
                        swapMoveDirections(marcherMove2, marcherSolutions[col.secondMarcher]);
                    }
                    
                    // If we're going to reinstruct our marchers, confirm the change in our marcher solutions
                    if (appliedSwap) {
                        SolverCoord swapTmp = marcherSolutions[col.secondMarcher].endPos;
                        marcherSolutions[col.secondMarcher].endPos = marcherSolutions[col.firstMarcher].endPos;
                        marcherSolutions[col.firstMarcher].endPos = swapTmp;
                    }
                            
                    collisionSpace.reinstructMarcher(col.firstMarcher, marcherMove1);
                    collisionSpace.reinstructMarcher(col.secondMarcher, marcherMove2);
                    
                    // If the collision was solved, go ahead and break from this
                    unsigned beatsWithoutCollision = collisionSpace.beatsBeforeCollisionBetweenMarchers(col.firstMarcher, col.secondMarcher);
                    if (beatsWithoutCollision >= beat) {
                        break;
                    }
                }
            }
            collisionPairs = collisionSpace.collectCollisionPairs();
        }
    }
}
    

}

// ========================
// ===-- FINAL SOLVER --===
// ========================

std::vector<CC_coord> runSolver(const CC_sheet& sheet1, const CC_sheet& sheet2) {
    std::vector<SolverCoord> startPositions;
    std::vector<SolverCoord> endPositions;
    fillPositionsFromSheet(sheet1, startPositions);
    fillPositionsFromSheet(sheet2, endPositions);
    
    Matrix<float> distances;
    distances = makeHungarianDistanceMatrix(startPositions, endPositions, 48);
    
    Munkres<float> solver;
    solver.solve(distances);
    
    std::vector<unsigned> assignments;
    for (auto i = 0; i < startPositions.size(); i++) {
        for (auto j = 0; j < startPositions.size(); j++) {
            if (distances(i, j) == 0) {
                assignments.push_back(j);
                break;
            }
        }
    }
    
    std::vector<MarcherSolution> marcherSolutions(assignments.size());
    std::vector<PathAxes> marcherMoveDirections(assignments.size());
    
    auto fieldWidth = 8*(100/5);
    auto fieldHeight = 32+32+20;
    unsigned maxBeats = 100;
    CollisionSpace collisionSpace(fieldWidth, fieldHeight, startPositions, maxBeats);
    
    try {
        for (auto i = 0; i < assignments.size(); i++) {
            marcherSolutions[i].startPos = startPositions[i];
            marcherSolutions[i].instruction = PATH_EWNS;
            
            setMarcherDestination(marcherSolutions[i], endPositions[assignments[i]], marcherMoveDirections[i]);
            refreshMarcherPathInCollisionSpace(collisionSpace, i, marcherSolutions[i], marcherMoveDirections[i]);
        }
        
        e7ChiuZamoraMalani::iterateSolution(marcherSolutions, collisionSpace, maxBeats);
    } catch (NotEnoughTimeException e) { // TODO: is this really what we want to do here?
        // Error; don't change anything
        for (auto i = 0; i < assignments.size(); i++) {
            marcherSolutions[i].endPos = endPositions[i];
        }
    }
    
    unsigned numOutstandingCollisions = collisionSpace.collectCollisionPairs().size();
    
    std::vector<CC_coord> finalPositions;
    for (auto i = 0; i < assignments.size(); i++) {
        finalPositions.push_back(SolverCoord::toShowSpace(marcherSolutions[i].endPos));
    }
    
    return finalPositions;
    
}

