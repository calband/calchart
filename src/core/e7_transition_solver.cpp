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




struct Collision {
    unsigned beat;
    unsigned firstMarcher;
    unsigned secondMarcher;
    CC_coord position;
};

struct MovingMarcher {
    unsigned waitBeats;
    unsigned moveDir1X;
    unsigned moveDir1Y;
    unsigned numStepsDir1;
    unsigned moveDir2X;
    unsigned moveDir2Y;
    unsigned numStepsDir2;
    unsigned startX;
    unsigned startY;
};


class CollisionSpace {
public:
    CollisionSpace(unsigned gridXSize, unsigned gridYSize, const std::vector<CC_coord>& marcherStartPositions, unsigned maxBeats);
//private:
    unsigned m_numBeats;
    std::vector<std::vector<std::vector<std::set<unsigned>>>> m_marcherGrid;
    std::vector<std::vector<std::map<unsigned, CC_coord>>> m_collisions;
    std::vector<MovingMarcher> m_marchers;
    
    void rollbackMarcherInstructions(unsigned which);
    void instructMarcher(unsigned which, const MovingMarcher& newInstructions);
    void reinstructMarcher(unsigned which, const MovingMarcher& newMarcherAnimation);
    
    const std::set<unsigned>& getMarchersAt(unsigned gridX, unsigned gridY, unsigned beat) const;
    std::set<unsigned>& getMarchersAt(unsigned gridX, unsigned gridY, unsigned beat);
    
    void _forgetCollision(unsigned firstMarcher, unsigned secondMarcher, unsigned beat);
    void _registerCollision(unsigned firstMarcher, unsigned secondMarcher, unsigned beat, CC_coord pos);
    
    void forgetCollision(unsigned firstMarcher, unsigned secondMarcher, unsigned beat);
    void registerCollision(unsigned firstMarcher, unsigned secondMarcher, unsigned beat, CC_coord pos);
    
    void removeMarcher(unsigned marcher, unsigned gridX, unsigned gridY, unsigned beat);
    void placeMarcher(unsigned marcher, unsigned gridX, unsigned gridY, unsigned beat);
};


CollisionSpace::CollisionSpace(unsigned gridXSize, unsigned gridYSize, const std::vector<CC_coord>& marcherStartPositions, unsigned maxBeats)
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
            .moveDir1X = 0,
            .moveDir1Y = 1,
            .numStepsDir1 = 0,
            .moveDir2X = 1,
            .moveDir2Y = 0,
            .numStepsDir2 = 0,
            .startX = (unsigned)Coord2Float(marcherStartPositions.at(marcherIndex).x),
            .startY = (unsigned)Coord2Float(marcherStartPositions.at(marcherIndex).y)
        };
        instructMarcher(marcherIndex, marcher);
    }
}

void CollisionSpace::rollbackMarcherInstructions(unsigned which) {
    auto& marcher = m_marchers[which];
    unsigned x = Coord2Float(marcher.startX);
    unsigned y = Coord2Float(marcher.startY);
    unsigned beat = 0;
    
    unsigned waitPhaseEnd = marcher.waitBeats;
    unsigned move1PhaseEnd = waitPhaseEnd + marcher.numStepsDir1;
    unsigned move2PhaseEnd = move1PhaseEnd + marcher.numStepsDir2;
    
    
    for (; beat < waitPhaseEnd; beat++) {
        removeMarcher(which, x, y, beat);
    }
    for (; beat < move1PhaseEnd; beat++) {
        x = x + Coord2Float(marcher.moveDir1X);
        y = y + Coord2Float(marcher.moveDir1Y);
        removeMarcher(which, x, y, beat);
    }
    for (; beat < move2PhaseEnd; beat++) {
        x = x + Coord2Float(marcher.moveDir2X);
        y = y + Coord2Float(marcher.moveDir2Y);
        removeMarcher(which, x, y, beat);
    }
    for (; beat < m_numBeats; beat++) {
        removeMarcher(which, x, y, beat);
    }
}

void CollisionSpace::instructMarcher(unsigned which, const MovingMarcher& newInstructions) {
    m_marchers[which] = newInstructions;
    auto& marcher = m_marchers[which];
    unsigned x = Coord2Float(marcher.startX);
    unsigned y = Coord2Float(marcher.startY);
    unsigned beat = 0;
    
    unsigned waitPhaseEnd = marcher.waitBeats;
    unsigned move1PhaseEnd = waitPhaseEnd + marcher.numStepsDir1;
    unsigned move2PhaseEnd = move1PhaseEnd + marcher.numStepsDir2;
    
    
    for (; beat < waitPhaseEnd; beat++) {
        placeMarcher(which, x, y, beat);
    }
    for (; beat < move1PhaseEnd; beat++) {
        x = x + Coord2Float(marcher.moveDir1X);
        y = y + Coord2Float(marcher.moveDir1Y);
        placeMarcher(which, x, y, beat);
    }
    for (; beat < move2PhaseEnd; beat++) {
        x = x + Coord2Float(marcher.moveDir2X);
        y = y + Coord2Float(marcher.moveDir2Y);
        placeMarcher(which, x, y, beat);
    }
    for (; beat < m_numBeats; beat++) {
        placeMarcher(which, x, y, beat);
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

void CollisionSpace::_registerCollision(unsigned int firstMarcher, unsigned int secondMarcher, unsigned int beat, CC_coord pos) {
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

void CollisionSpace::registerCollision(unsigned int firstMarcher, unsigned int secondMarcher, unsigned int beat, CC_coord pos) {
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














void fillPositionsFromSheet(const CC_sheet& sheet, std::vector<CC_coord>& positions) {
    auto points = sheet.GetPoints();
    for (auto i = 0; i < points.size(); i++) {
        positions.push_back(points[i].GetPos());
    }
}

Matrix<float> makeHungarianDistanceMatrix(const std::vector<CC_coord>& startPositions, const std::vector<CC_coord>& endPositions, unsigned numBeats) {
    size_t numMarchers = startPositions.size();
    Matrix<float> matrix = Matrix<float>(numMarchers, numMarchers);
    for (auto i = 0; i < startPositions.size(); i++) {
        for (auto j = 0; j < endPositions.size(); j++) {
            auto diff = startPositions.at(i) - endPositions.at(j);
            auto manhattanDist = Coord2Float(abs(diff.x) + abs(diff.y));
            if (manhattanDist > numBeats) {
                matrix(i, j) = std::numeric_limits<float>::infinity();
            } else {
                matrix(i, j) = manhattanDist;
            }
        }
    }
    return matrix;
}

std::vector<CC_coord> runSolver(const CC_sheet& sheet1, const CC_sheet& sheet2) {
    std::vector<CC_coord> startPositions;
    std::vector<CC_coord> endPositions;
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
    
    std::vector<CC_coord> finalPositions;
    for (auto i = 0; i < assignments.size(); i++) {
        finalPositions.push_back(endPositions[assignments[i]]);
    }
    
    auto fieldWidth = 8*(100/5);
    auto fieldHeight = 32+32+20;
    
    
    
    return finalPositions;
    
}


//
//void groupAlgorithm_hungarian3() {
//    
//}
//
//
//void minimizeCollisions_hungarian3(unsigned numBeats) {
//    const unsigned numFixIterations = 4;
//    for (auto solvedBeats = 0; solvedBeats < numBeats; solvedBeats++) {
//        for (auto fixIteration = 0; fixIteration < numFixIterations; fixIteration++) {
//            // Find collisions HERE
//            if (numColisions == 0) {
//                break;
//            }
//            // Switch Assignments HERE
//            // Change waits and directions HERE
//        }
//    }
//}










std::pair<float, float> stepsInEachDir(CC_coord startPos, CC_coord endPos, CC_coord stepVector1, CC_coord stepVector2) {
    // startPos + (numStepsLeg1 * stepVector1) + (numStepsLeg2 * stepVector2) = endPos
    // (numStepsLeg1 * stepVector1) + (numStepsLeg2 * stepVector2) = endPos - startPos = moveVector
    CC_coord moveVector = endPos - startPos;
    
    // Split the equation above into one equation for each vector component (one for x components and one for y)
    // Focus first on the x components
    // (numStepsLeg1 * stepVector1.x) + (numStepsLeg2 * stepVector2.x) = moveVector.x
    // (numStepsLeg1 * stepVector1.x) = moveVector.x - (numStepsLeg2 * stepVector2.x)
    // numStepsLeg1 = (moveVector.x / stepVector1.x) - (stepVector2.x / stepVector1.x) * numStepsLeg2 = a - b * numStepsLeg2
    float a = Coord2Float(moveVector.x) / Coord2Float(stepVector1.x);
    float b = Coord2Float(stepVector2.x) / Coord2Float(stepVector1.x);
    
    // Next consider y components
    // (numStepsLeg1 * stepVector1.y) + (numStepsLeg2 * stepVector2.y) = moveVector.y
    // ((a - b * numStepsLeg2) * stepVector1.y) + (numStepsLeg2 * stepVector2.y) = moveVector.y
    // (a * stepVector1.y) - (numStepsLeg2 * b * stepVector1.y) + (numStepsLeg2 * stepVector2.y) = moveVector.y
    // numStepsLeg2 * (stepVector2.y - b * stepVector1.y) = moveVector.y - (a * stepVector1.y)
    // numStepsLeg2 = (moveVector.y - (a * stepVector1.y)) / (stepVector2.y - (b * stepVector1.y))
    float numStepsLeg2 = (moveVector.y - (a * stepVector1.y)) / (stepVector2.y - (b * stepVector1.y));
    
    float numStepsLeg1 = a - (b * numStepsLeg2);
    
    return { numStepsLeg1, numStepsLeg2 };
}



//
//std::vector<Collision> findCollisions(const std::vector<>& marchers) {
//    // Add each marcher to the collision space, recording collisions as you go
//    // Return those collisions
//}

