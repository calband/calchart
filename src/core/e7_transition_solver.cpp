//
//  e7_transition_solver.cpp
//  CalChart
//
//  Created by Kevin Durand on 10/12/16.
//
//

#include <math.h>
#include <limits>
#include <fstream>
#include <algorithm>

#include "e7_transition_solver.h"
#include "munkres.h"

namespace CalChart {

#pragma mark - Shared

// ==================
// ===-- SHARED --===
// ==================

/*!
 * @brief Represents a coordinate in the TransitionSolver.
 * @discussion In the TransitionSolver, a marcher moves at a rate of
 * one unit x or one unit y in each beat. These coordinates, however,
 * can be scaled if the rate of time is scaled as well. For example, if
 * one unit x and one unit y are scaled to represent the distance that
 * a marcher moves over two beats, then one 'beat' in the solver should
 * be scaled to represent two actual beats.
 */
class SolverCoord {
public:
    static const unsigned kFieldWidthInSteps = (100 / 5) * 8; // 100 yards, 8 steps per yard
    static const unsigned kFieldHeightInSteps = 32 + 20 + 32; // 32 steps sideline-to-hash, 20 steps hash-to-hash

    int32_t x;
    int32_t y;

    SolverCoord(CalChart::Coord showCoord, unsigned fieldWidthInSteps = kFieldWidthInSteps, unsigned fieldHeightInSteps = kFieldHeightInSteps)
        : x(CoordUnits2Float(showCoord.x) + (fieldWidthInSteps / 2))
        , y(CoordUnits2Float(showCoord.y) + (fieldHeightInSteps / 2)){};
    SolverCoord(int32_t x = 0, int32_t y = 0)
        : x(x)
        , y(y){};

    auto toShowSpace(unsigned fieldWidthInSteps = kFieldWidthInSteps, unsigned fieldHeightInSteps = kFieldHeightInSteps)
    {
        CalChart::Coord showCoord;
        showCoord.x = Float2CoordUnits(((float)this->x) - (fieldWidthInSteps / 2));
        showCoord.y = Float2CoordUnits(((float)this->y) - (fieldHeightInSteps / 2));
        return showCoord;
    }

    static SolverCoord fromShowSpace(CalChart::Coord showCoord, unsigned fieldWidthInSteps = kFieldWidthInSteps, unsigned fieldHeightInSteps = kFieldHeightInSteps)
    {
        return SolverCoord(showCoord, fieldWidthInSteps, fieldHeightInSteps);
    }
    static auto toShowSpace(SolverCoord solverCoord, unsigned fieldWidthInSteps = kFieldWidthInSteps, unsigned fieldHeightInSteps = kFieldHeightInSteps)
    {
        return solverCoord.toShowSpace(fieldWidthInSteps, fieldHeightInSteps);
    }

    bool operator==(const SolverCoord& other) const
    {
        return (this->x == other.x && this->y == other.y);
    }

    bool operator==(int32_t scalar) const
    {
        return (this->x == scalar && this->y == scalar);
    }

    SolverCoord& operator=(const SolverCoord& other)
    {
        this->x = other.x;
        this->y = other.y;
        return *this;
    }

    SolverCoord& operator+=(const SolverCoord& other)
    {
        this->x += other.x;
        this->y += other.y;
        return *this;
    }

    SolverCoord& operator-=(const SolverCoord& other)
    {
        this->x -= other.x;
        this->y -= other.y;
        return *this;
    }

    SolverCoord& operator*=(int32_t scalar)
    {
        this->x *= scalar;
        this->y *= scalar;
        return *this;
    }

    SolverCoord& operator/=(int32_t scalar)
    {
        this->x /= scalar;
        this->y /= scalar;
        return *this;
    }

    SolverCoord& operator+=(int32_t scalar)
    {
        this->x += scalar;
        this->y += scalar;
        return *this;
    }

    SolverCoord& operator-=(int32_t scalar)
    {
        this->x -= scalar;
        this->y -= scalar;
        return *this;
    }

    SolverCoord operator+(const SolverCoord& other) const
    {
        SolverCoord newCoord = *this;
        newCoord += other;
        return newCoord;
    }

    SolverCoord operator-(const SolverCoord& other) const
    {
        SolverCoord newCoord = *this;
        newCoord -= other;
        return newCoord;
    }

    SolverCoord operator*(int32_t scalar) const
    {
        SolverCoord newCoord = *this;
        newCoord *= scalar;
        return newCoord;
    }

    SolverCoord operator/(int32_t scalar) const
    {
        SolverCoord newCoord = *this;
        newCoord /= scalar;
        return newCoord;
    }

    SolverCoord operator+(int32_t scalar) const
    {
        SolverCoord newCoord = *this;
        newCoord += scalar;
        return newCoord;
    }

    SolverCoord operator-(int32_t scalar) const
    {
        SolverCoord newCoord = *this;
        newCoord -= scalar;
        return newCoord;
    }

    SolverCoord operator-() const
    {
        return (*this) * -1;
    }
};

/*!
 * @brief Represents a collision within the collision space.
 */
struct Collision {
    /*!
     * @brief The beat on which the collision occurred.
     */
    unsigned beat;

    /*!
     * @brief The index of the first marcher involved in the collision.
     */
    unsigned firstMarcher;

    /*!
     * @brief The index of the second marcher involved in the collision.
     */
    unsigned secondMarcher;

    /*!
     * @brief The coordinate at which the collision occurred, within the coordinate system
     * used by the Transition Solver.
     */
    SolverCoord position;

    bool operator==(const Collision& other) const
    {
        return (beat == other.beat && ((firstMarcher == other.firstMarcher && secondMarcher == other.secondMarcher) || (firstMarcher == other.secondMarcher && secondMarcher == other.firstMarcher)) && position == other.position);
    }

    bool operator!=(const Collision& other) const
    {
        return !(*this == other);
    }
};

/*!
 * @brief Contains the instructions for how a marcher will move during a
 * transition.
 * @detail Movements are broken down into a maximum of four segments:
 * a wait segment, followed by two move segments, followed by a final wait segment.
 * The durations of the first three segments are provided explicitly, but the
 * duration of the final wait segment is given implicit, and fills whatever time
 * remains during the transition starting from the end of the second move segment.
 * During each move segment, the marcher moves in a straight line, and the amount
 * that the marcher moves on each beat is given by the step vector associated with
 * the segment.
 */
struct MovingMarcher {
    /*!
     * @brief The number of beats that the marcher will initially wait before moving.
     */
    unsigned waitBeats;

    /*!
     * @brief Vectors defining how far a marcher will move on each beat during each
     * of its two movement phases.
     * @detail Note that the marcher should not move more than one unit along any
     * given axis in a single beat. Otherwise, the marcher may travel through other marchers
     * without flagging a collision in the collision space.
     */
    std::pair<SolverCoord, SolverCoord> stepVectors;

    /*!
     * @brief Defines the duration of each of the marcher's two movement phases.
     */
    std::pair<unsigned, unsigned> numSteps;

    /*!
     * @brief Defines the location at which the marcher's movement begins (which
     * is analogous to the marcher's location on beat zero).
     */
    SolverCoord startPos;

    bool operator==(const MovingMarcher& other) const
    {
        return waitBeats == other.waitBeats && stepVectors == other.stepVectors && numSteps == other.numSteps && startPos == other.startPos;
    }

    bool operator!=(const MovingMarcher& other) const
    {
        return !(*this == other);
    }
};

/*!
 * @brief Describes the beat numbers on which a marcher transitions
 * between its various phases of movement.
 */
struct MarcherMoveSchedule {
    /*!
     * @brief This is the last beat on which the marcher should wait
     * before it begins to move.
     */
    unsigned lastBeatOfWait;

    /*!
     * @brief This is the last beat on which the marcher should move
     * along its first step vector.
     */
    unsigned lastBeatOfFirstMove;

    /*!
     * @brief This is the last beat on which the marcher should move
     * along its second step vector.
     */
    unsigned lastBeatOfSecondMove;
};

/*!
 * @brief Represents the state of a marcher at some beat during
 * a transition.
 */
struct MarcherSnapshot {
    SolverCoord currentPosition;
    unsigned currentBeat;
};

class CollisionSpace {
public:
    CollisionSpace(unsigned gridXSize, unsigned gridYSize, const std::vector<SolverCoord>& marcherStartPositions, unsigned maxBeats);

private:
    // ------------------------------
    // --- ACCURATE FOR ALL BEATS ---
    // ------------------------------

    /*!
     * @brief The maximum number of beats allowed in the transition that is monitored
     * by this collision space.
     * @discussion The first beat in the transition is the zeroth beat. After that,
     * the number of beats of movement in the transition is m_numBeats.
     */
    unsigned m_numBeats;

    /*!
     * @brief Associates each marcher index with the instruction for that marcher
     * within the collision space.
     */
    std::vector<MovingMarcher> m_marchers;

    /*!
     * @brief Contains the indices of all marchers who, given their current instruction
     * set, will not make it to their destinations within the current number of beats.
     */
    std::set<unsigned> m_incompleteTransitions;

    /*!
     * @brief Records the current clip beat.
     * @discussion The clip beat is used to focus the collision space on a reduced number
     * of beats, so that collisions can be detected in that range without needing to
     * calculate and track all collisions in the entire timespan that is covered by the
     * collision space.
     */
    unsigned m_clipBeat;

    /*!
     * @brief A set containing the indices of all currently disabled marchers.
     * @detail When a marcher is disabled, all of its collisions (with the exception
     * of those occurring on the zeroth beat) are disregarded.
     */
    std::set<unsigned> m_disabledMarchers;

    /*!
     * @brief A cached list of the move schedule for each marcher.
     */
    mutable std::vector<MarcherMoveSchedule> m_moveSchedules;

    // ------------------------------------------------
    // --- ACCURATE UP TO (AND INCLUDING) CLIP BEAT ---
    // ------------------------------------------------

    /*!
     * @brief Records all of the collisions occuring at each grid position, on each beat.
     * @detail m_collisions[i][j][t], if it exists, contains the site of the collision
     * between the marchers with indices i and j on beat t. If no entry exists for time
     * t in m_collisions[i][j], then there is no collision between marchers i and j
     * on that beat. Note that this is only accurate up to the clip beat; no collisions
     * are recorded for any beat after the clip beat.
     */
    mutable std::vector<std::vector<std::map<unsigned, SolverCoord> > > m_collisions;

    /*!
     * @brief Records all of the marchers that are present at each grid position, on each beat.
     * @detail m_marcherGrid[x][y][t] is a set containing the indices of all  marchers
     * that reside at grid location (x, y) on beat t.
     * m_marcherGrid is reliable up to (and including) the clip beat. All locations are empty
     * for all beats after the clip beat.
     */
    mutable std::vector<std::vector<std::vector<std::set<unsigned> > > > m_marcherGrid;

    /*!
     * @brief Captures the state of all marchers on the current clip beat (that is, where they
     * are, and where they are going).
     * @detail Note that each marcher is associated with its own current beat. When you change
     * the clip beat, all marchers should be updated until they maintain the same current beat.
     * The exception is disabled marchers, which keep a current beat of zero regardless of the
     * clip beat for the collision space.
     */
    mutable std::vector<MarcherSnapshot> m_clippedMarchers;

    // ------------------------------------------
    // --- ACCURATE WHILE CACHE IS MAINTAINED ---
    // ------------------------------------------

    /*!
     * @brief Denotes whether or not the current value of m_cachedCollisionPairs
     * is accurate.
     * @detail A value of true indicates that m_cachedCollisionPairs is not
     * currently accurate. A value of false indicates that m_cachedCollisionPairs
     * is accurate.
     */
    mutable bool m_cachedCollisionsNeedRefresh;

    /*!
     * @brief If m_cachedCollisionPairs is accurate, then this value will indicate
     * the beat for which it contains the collisions.
     * @detail The value is undefined if the value of m_cachedCollisionPairs is not
     * accurate. Use m_cachedCollisionsNeedRefresh to determine whether or not
     * m_cachedCollisionPairs is accurate.
     */
    mutable unsigned m_cachedCollisionsBeat;

    /*!
     * @brief Unless it needs to be refreshed, contains a list of all of the
     * collisions occurring up to and including a particular beat.
     * @detail This will keep track of all collisions occurring up to and
     * including m_cachedCollisionsBeat. To identify whether or not this value
     * is accurate, check m_cachedCollisionsNeedRefresh.
     */
    mutable std::vector<Collision> m_cachedCollisionPairs;

    /*!
     * @brief Denotes whether or not the current value of m_cachedCollisionsForClipBeat
     * is accurate.
     * @detail A value of true indicates that m_cachedCollisionsForClipBeat is not
     * currently accurate. A value of false indicates that m_cachedCollisionsForClipBeat
     * is currently accurate.
     */
    mutable bool m_clipBeatCollisionsNeedRefresh;

    /*!
     * @brief Unless it needs to be refreshed, contains a list of all of the
     * collisions occurring up to and including the current clip beat.
     * @detail To identify whether or not this value is currently accurate, check
     * m_clipBeatCollisionsNeedRefresh.
     */
    mutable std::vector<Collision> m_cachedCollisionsForClipBeat;

public:
    /*!
     * @brief Identifies whether or not the transition is completely solved.
     * @detail The criteria for a transition to be solved are as follows:
     * - All marchers are enabled, and are thus accounted for in the transition
     * - The transition is not clipped short (meaning the clip beat is equal to the number
     *   of beats available for the transition)
     * - All marchers reach their destinations within the number of beats allowed for
     *   the transition.
     * - There are no collisions during the transition
     * @return True if the transition is completely solved; false otherwise.
     */
    bool isSolved() const;

    /*!
     * @brief Disables the marcher with the given index, so that it is removed from the
     * collision space for all beats (with the exception of beat zero).
     * @param which The index of the marcher to disable.
     */
    void disableMarcher(unsigned which);
    /*!
     * @brief Enables the marcher with the given index, ensuring that it is accounted for
     * in the collision space for all beats up to and including the current clip beat.
     * @param The index of the marcher to enable.
     */
    void enableMarcher(unsigned which);

    /*!
     * @brief Issues a new set of instructions to the specified marcher.
     * @detail A marcher can be reinstructed whether or not it is currently enabled.
     * @param which The index of the marcher to assign a new set of instructions to.
     * @param newMarcherAnimation The new instructions to assign to the marcher.
     */
    void reinstructMarcher(unsigned which, const MovingMarcher& newMarcherAnimation);
    /*!
     * @brief Sets the maximum number of beats that the collision space will actively
     * focus on in the transition.
     * @detail The collision space will track collisions up to and including the current
     * clip beat. When the clip beat is set, the collision space ignores any unneccessary
     * calculations that are relevant to beats after the clip beat. Setting a clip
     * beat is good for automatically filtering collisions, and for speeding up calculations.
     * @param The new clip beat. Regardless of how large this value is, it will always get
     * capped at the number of beats in the transition.
     */
    void clipToBeat(unsigned clipBeat);

    /*!
     * @brief Gets a set containing the indices of marchers who cannot reach their destinations
     * before the end of the transition, given their current instructions.
     * @return A set containing the indices of marchers who cannot reach their destinations
     * in the allotted amount of time for the transition.
     */
    std::set<unsigned> getMarchersWithIncompleteTransitions() const;

    /*!
     * @brief Collects together all collisions occuring up to and including the current clip
     * beat, and returns a filtered version such that, for any pair of marchers, only the first
     * collision between them is included.
     * @return A filtered list of collisions containing at most one collision per pair of marchers;
     * each collision represents the first collision occuring between a pair of marchers.
     */
    std::vector<Collision> collectCollisionPairs() const;
    /*!
     * @brief Collects together all collisions occuring up to and including the specified
     * beat, and returns a filtered version such that, for any pair of marchers, only the first
     * collision between them is included.
     * @param The latest beat for which collisions should be collected.
     * @return A filtered list of collisions containing at most one collision per pair of marchers;
     * each collision represents the first collision occuring between a pair of marchers.
     */
    std::vector<Collision> collectCollisionPairs(unsigned maxBeat) const;
    /*!
     * @brief Returns the number of beats between the beginning of the transition and the
     * first collision between a particular pair of marchers.
     * @param marcher1 The index of the first marcher involved in the collisions.
     * @param marcher2 The index of the second marcher involved in the collisions.
     * @return The number of beats between the beginning of the transition and the first
     * collision between the two specified marchers.
     */
    unsigned beatsBeforeCollisionBetweenMarchers(unsigned marcher1, unsigned marcher2) const;
    /*!
     * @brief Returns the earliest beat at which all marchers have completed
     * their movements.
     * @detail The value returned by this method is capped to the current clip beat.
     * If any marchers are still in the process of moving at the current clip beat,
     * then this value will not return an accurate result.
     * @return The earliest beat at which all marchers have completed their movements.
     */
    unsigned firstBeatAfterMovment() const;

    /*!
     * @brief Returns the movement instruction that is currently assigned to the marcher
     * with the given index.
     * @param marcherIndex The index of the marcher whose instruction will be accessed.
     * @return The movement instruction that is currently assigned to the specified
     * marcher.
     */
    const MovingMarcher& getMarcherInstruction(unsigned marcherIndex) const;

private:
    std::vector<Collision> _collectCollisionPairs(unsigned maxBeat) const;

    /*!
     * @brief Clips the movement schedule for a marcher, such no movement
     * occurs on or after the specified beat.
     * @param movement The original movement schedule for a marcher. This 
     * is the schedule that will be clipped.
     * @param beat The beat to which the schedule shall be clipped.
     * @return The adjusted movement schedule, with no movement phase persisting
     * past the specified beat number.
     */
    MarcherMoveSchedule clipMarcherSchedule(const MarcherMoveSchedule& movement, unsigned beat) const;
    /*!
     * @brief Clips the movement schedule for a marcher, such no movement
     * occurs on or after the specified beat.
     * @param which The index of the marcher whose movement schedule should
     * be clipped.
     * @param beat The beat to which the schedule shall be clipped.
     * @return The adjusted movement schedule, with no movement phase persisting
     * past the specified beat number.
     */
    MarcherMoveSchedule getClippedMarcherSchedule(unsigned which, unsigned beat) const;

    /*!
     * @brief Advances the current snapshot for the marcher with the given index
     * by the designated number of beats.
     * @param which The index of the marcher whose snapshot should be advanced.
     * @param numBeats The number of beats by which the marcher's snapshot should
     * be advanced.
     */
    void advanceMarcher(unsigned which, unsigned numBeats);
    /*!
     * @brief Regresses the snapshot for the designated marcher to beat zero,
     * and removes the marcher from any position in the collision space at any
     * beat after that.
     * @param The index of the marcher that should be retracted from the collision
     * space for all beats after beat zero.
     */
    void rollbackMarcherInstructions(unsigned which);
    /*!
     * @brief Issues an instruction to a marcher that currently has no instruction.
     * @detail This should only be used when first setting up a marcher, or right
     * after rolling back a marcher. It assumes that the marcher's current snapshot
     * is positioned at beat zero, and it will advance the snapshot according to the
     * new instruction until the marcher reaches the current clip beat.
     * @param which The index of the marcher that should accept the new instruction.
     * @param newInstruction The new instruction to assign to the marcher.
     */
    void instructMarcher(unsigned which, const MovingMarcher& newInstructions);

    /*!
     * @brief Returns the indices of all marchers present at a particular
     * location at a particular
     * time.
     * @param gridX The x coordinate of the position containing the returned
     * marchers, where the coordinate system matches the one that is understood
     * by SolverCoords.
     * @param gridY The y coordinate of the position containing the returned
     * marchers, where the coordinate system matches the one that is understood
     * by SolverCoords.
     * @param beat The beat on which to check the occupants of the provided location.
     * @return A set containing the indices of all marchers present at location
     * (gridX, gridY) on provided beat.
     */
    const std::set<unsigned>& getMarchersAt(unsigned gridX, unsigned gridY, unsigned beat) const;
    /*!
     * @brief Returns the indices of all marchers present at a particular 
     * location at a particular
     * time.
     * @param gridX The x coordinate of the position containing the returned
     * marchers, where the coordinate system matches the one that is understood
     * by SolverCoords.
     * @param gridY The y coordinate of the position containing the returned
     * marchers, where the coordinate system matches the one that is understood
     * by SolverCoords.
     * @param beat The beat on which to check the occupants of the provided location.
     * @return A set containing the indices of all marchers present at location
     * (gridX, gridY) on provided beat.
     */
    std::set<unsigned>& getMarchersAt(unsigned gridX, unsigned gridY, unsigned beat);

    void _forgetCollision(unsigned firstMarcher, unsigned secondMarcher, unsigned beat);
    void _registerCollision(unsigned firstMarcher, unsigned secondMarcher, unsigned beat, SolverCoord pos);

    /*!
     * @brief Unregisters any collisions between two marchers taking place
     * on a particular beat.
     * @param firstMarcher The index of one of the first marcher that is not
     * involved in a collision.
     * @param secondMarcher The index of the second marcher that is not involved
     * in a collision.
     * @param beat The beat from which to remove any record of collisions between
     * the specified two marchers.
     */
    void forgetCollision(unsigned firstMarcher, unsigned secondMarcher, unsigned beat);
    /*!
     * @brief Registers a collision between two marchers at a particular
     * location on a particular beat.
     * @param firstMarcher The index of one of the marchers involved in the
     * collision.
     * @param secondMarcher The index of the other marcher involved in the collision.
     * @param beat The beat on which the collision occurs.
     * @param pos The position at which the collision occurs.
     */
    void registerCollision(unsigned firstMarcher, unsigned secondMarcher, unsigned beat, SolverCoord pos);

    /*!
     * @brief Within the collision space, registers a marcher as being absent
     * from a particular location at a particular time.
     * @param marcher The index of the marcher being placed in the collision space
     * at a particular moment.
     * @param gridX The x coordinate from which the marcher is being removed, where the
     * coordinate system matches the coordinate system understood by SolverCoords.
     * @param gridY The y coordinate from which the marcher is being removed, where the
     * coordinate system matches the coordinate system understood by SolverCoords.
     * @param moveVectorFromPrevBeat Identifies the movement that the marcher made
     * during the beat before this one, which originally brought it to this location from
     * which it is now being removed.
     */
    void removeMarcher(unsigned marcher, unsigned gridX, unsigned gridY, unsigned beat, const SolverCoord& moveVectorFromPrevBeat);
    /*!
     * @brief Within the collision space, registers the location of a particular
     * marcher at a particular time.
     * @param marcher The index of the marcher being placed in the collision space
     * at a particular moment.
     * @param gridX The x coordinate at which the marcher is being placed, where the 
     * coordinate system matches the coordinate system understood by SolverCoords.
     * @param gridY The y coordinate at which the marcher is being placed, where the
     * coordinate system matches the coordinate system understood by SolverCoords.
     * @param moveVectorFromPrevBeat Identifies the movement that the marcher made
     * during the last beat which brought it to the location where it is now being placed.
     */
    void placeMarcher(unsigned marcher, unsigned gridX, unsigned gridY, unsigned beat, const SolverCoord& moveVectorFromPrevBeat);

    /*!
     * @brief Acquires a list of all marchers that travel from a specific
     * start location on a specific beat to a specific destination on the
     * next beat.
     * @param startBeat The beat during which the marchers are at the specified
     * start coordinate.
     * @param firstCoord The coordinate at which all of the marcher start.
     * @param secondCoord The coordinate to which all of the marchers move by
     * the next beat.
     * @return A set of the indices of all marchers who start at firstCoord on
     * startBeat and end on secondCoord at time (startBeat + 1).
     */
    std::set<unsigned> getMarchersWithMovePattern(unsigned startBeat, SolverCoord firstCoord, SolverCoord secondCoord) const;
};

CollisionSpace::CollisionSpace(unsigned gridXSize, unsigned gridYSize, const std::vector<SolverCoord>& marcherStartPositions, unsigned maxBeats)
    : m_numBeats(maxBeats)
    , m_clipBeat(m_numBeats)
    , m_cachedCollisionsNeedRefresh(true)
    , m_cachedCollisionsBeat(0)
    , m_clipBeatCollisionsNeedRefresh(true)
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
    m_moveSchedules.resize(marcherStartPositions.size());
    for (unsigned marcherIndex = 0; marcherIndex < marcherStartPositions.size(); marcherIndex++) {
        MovingMarcher marcher = {
            0,
            { SolverCoord(), SolverCoord() },
            { 0, 0 },
            marcherStartPositions.at(marcherIndex)
        };
        MarcherMoveSchedule moveBreakdown = {
            0,
            0,
            0
        };
        m_clippedMarchers[marcherIndex] = {
            marcher.startPos,
            0
        };
        m_moveSchedules[marcherIndex] = moveBreakdown;
        placeMarcher(marcherIndex, marcher.startPos.x, marcher.startPos.y, 0, SolverCoord(0, 0));
        instructMarcher(marcherIndex, marcher);
    }
}

bool CollisionSpace::isSolved() const
{
    return (m_clipBeat == m_numBeats) && (collectCollisionPairs().size() == 0) && (getMarchersWithIncompleteTransitions().size() == 0 && m_disabledMarchers.size() == 0);
}

std::set<unsigned> CollisionSpace::getMarchersWithIncompleteTransitions() const
{
    return m_incompleteTransitions;
}

void CollisionSpace::disableMarcher(unsigned which)
{
    if (m_disabledMarchers.find(which) == m_disabledMarchers.end()) {
        m_disabledMarchers.insert(which);
        reinstructMarcher(which, m_marchers[which]);
    }
}

void CollisionSpace::enableMarcher(unsigned which)
{
    if (m_disabledMarchers.find(which) != m_disabledMarchers.end()) {
        m_disabledMarchers.erase(which);
        reinstructMarcher(which, m_marchers[which]);
    }
}

std::vector<Collision> CollisionSpace::collectCollisionPairs() const
{
    if (m_clipBeatCollisionsNeedRefresh) {
        m_cachedCollisionsForClipBeat = _collectCollisionPairs(m_clipBeat);
        m_clipBeatCollisionsNeedRefresh = false;
    }
    return m_cachedCollisionsForClipBeat;
}

std::vector<Collision> CollisionSpace::collectCollisionPairs(unsigned maxBeat) const
{
    if (maxBeat > m_clipBeat) {
        maxBeat = m_clipBeat;
    }

    if (maxBeat == m_clipBeat) {
        return collectCollisionPairs();
    }

    if (maxBeat != m_cachedCollisionsBeat || m_cachedCollisionsNeedRefresh) {
        m_cachedCollisionPairs = _collectCollisionPairs(maxBeat);
        m_cachedCollisionsBeat = maxBeat;
        m_cachedCollisionsNeedRefresh = false;
    }

    return m_cachedCollisionPairs;
}

std::vector<Collision> CollisionSpace::_collectCollisionPairs(unsigned maxBeat) const
{
    std::vector<Collision> collisions;
    for (unsigned i = 0; i < m_marchers.size(); i++) {
        for (unsigned j = i + 1; j < m_marchers.size(); j++) {
            unsigned firstColBeat = beatsBeforeCollisionBetweenMarchers(i, j);
            if (firstColBeat <= maxBeat) {
                collisions.push_back({ firstColBeat, i, j, m_collisions.at(i).at(j).at(firstColBeat).toShowSpace((unsigned)m_marcherGrid.size(), (unsigned)m_marcherGrid[0].size()) });
            }
        }
    }
    return collisions;
}

unsigned CollisionSpace::beatsBeforeCollisionBetweenMarchers(unsigned marcher1, unsigned marcher2) const
{
    auto& collisionsBetweenMarchers = m_collisions[marcher1][marcher2];
    unsigned firstColBeat = m_numBeats + 1;
    for (auto colIter = collisionsBetweenMarchers.begin(); colIter != collisionsBetweenMarchers.end(); colIter++) {
        if (colIter->first < firstColBeat) {
            firstColBeat = colIter->first;
        }
    }
    return firstColBeat;
}

unsigned CollisionSpace::firstBeatAfterMovment() const
{
    unsigned maxMoveBeat = 0;

    for (unsigned i = 0; i < m_marchers.size(); i++) {
        MarcherMoveSchedule moveSchedule;

        moveSchedule = getClippedMarcherSchedule(i, m_clipBeat);

        if (moveSchedule.lastBeatOfSecondMove > maxMoveBeat) {
            maxMoveBeat = moveSchedule.lastBeatOfSecondMove;
        }
    }

    return maxMoveBeat;
}

const MovingMarcher& CollisionSpace::getMarcherInstruction(unsigned marcherIndex) const
{
    return m_marchers.at(marcherIndex);
}

MarcherMoveSchedule CollisionSpace::clipMarcherSchedule(const MarcherMoveSchedule& movement, unsigned beat) const
{
    MarcherMoveSchedule result = {
        std::min(movement.lastBeatOfWait, beat),
        std::min(movement.lastBeatOfFirstMove, beat),
        std::min(movement.lastBeatOfSecondMove, beat)
    };
    return result;
}

MarcherMoveSchedule CollisionSpace::getClippedMarcherSchedule(unsigned which, unsigned beat) const
{
    return clipMarcherSchedule(m_moveSchedules[which], beat);
}

void CollisionSpace::rollbackMarcherInstructions(unsigned which)
{
    auto& marcher = m_marchers[which];
    SolverCoord pos = marcher.startPos;
    unsigned beat = 0;

    MarcherMoveSchedule marcherMovements = getClippedMarcherSchedule(which, m_clippedMarchers[which].currentBeat);

    m_incompleteTransitions.erase(which);

    for (; beat <= marcherMovements.lastBeatOfWait; beat++) {
        removeMarcher(which, pos.x, pos.y, beat, SolverCoord(0, 0));
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
        removeMarcher(which, pos.x, pos.y, beat, SolverCoord(0, 0));
    }

    // After a rollback, we still want our marcher in its first position
    placeMarcher(which, marcher.startPos.x, marcher.startPos.y, 0, SolverCoord(0, 0));
    m_clippedMarchers[which].currentBeat = 0;
    m_clippedMarchers[which].currentPosition = marcher.startPos;
}

void CollisionSpace::instructMarcher(unsigned which, const MovingMarcher& newInstructions)
{
    m_marchers[which] = newInstructions;
    auto& marcher = m_marchers[which];

    MarcherMoveSchedule& moveSchedule = m_moveSchedules[which];

    moveSchedule.lastBeatOfWait = marcher.waitBeats;
    moveSchedule.lastBeatOfFirstMove = moveSchedule.lastBeatOfWait + marcher.numSteps.first;
    moveSchedule.lastBeatOfSecondMove = moveSchedule.lastBeatOfFirstMove + marcher.numSteps.second;

    if (moveSchedule.lastBeatOfSecondMove > m_numBeats) {
        m_incompleteTransitions.insert(which);
    }

    advanceMarcher(which, m_clipBeat);
}

void CollisionSpace::advanceMarcher(unsigned which, unsigned numBeats)
{
    auto& marcher = m_marchers[which];
    auto moveSchedule = getClippedMarcherSchedule(which, m_clipBeat);

    MarcherSnapshot& clippedMarcher = m_clippedMarchers[which];
    SolverCoord pos = clippedMarcher.currentPosition;
    unsigned beat = clippedMarcher.currentBeat + 1;

    for (; beat <= moveSchedule.lastBeatOfWait && numBeats > 0; beat++, numBeats--) {
        placeMarcher(which, pos.x, pos.y, beat, SolverCoord(0, 0));
    }
    for (; beat <= moveSchedule.lastBeatOfFirstMove && numBeats > 0; beat++, numBeats--) {
        pos += marcher.stepVectors.first;
        placeMarcher(which, pos.x, pos.y, beat, marcher.stepVectors.first);
    }
    for (; beat <= moveSchedule.lastBeatOfSecondMove && numBeats > 0; beat++, numBeats--) {
        pos += marcher.stepVectors.second;
        placeMarcher(which, pos.x, pos.y, beat, marcher.stepVectors.second);
    }
    for (; beat <= m_clipBeat && numBeats > 0; beat++, numBeats--) {
        placeMarcher(which, pos.x, pos.y, beat, SolverCoord(0, 0));
    }

    clippedMarcher.currentPosition = pos;
    clippedMarcher.currentBeat = beat - 1;
}

void CollisionSpace::reinstructMarcher(unsigned which, const MovingMarcher& newMarcherAnimation)
{
    m_clipBeatCollisionsNeedRefresh = true;
    m_cachedCollisionsNeedRefresh = true;

    rollbackMarcherInstructions(which);

    if (m_disabledMarchers.find(which) != m_disabledMarchers.end()) {
        m_marchers[which] = newMarcherAnimation;
    }
    else {
        instructMarcher(which, newMarcherAnimation);
    }
}

void CollisionSpace::clipToBeat(unsigned clipBeat)
{
    if (clipBeat > m_numBeats) {
        clipBeat = m_numBeats;
    }

    if (clipBeat > m_clipBeat) {
        for (unsigned i = 0; i < m_marchers.size(); i++) {
            advanceMarcher(i, clipBeat - m_clipBeat);
        }
    }
    else {
        for (unsigned i = 0; i < m_marchers.size(); i++) {
            reinstructMarcher(i, m_marchers[i]);
        }
    }

    if (clipBeat != m_clipBeat) {
        if (m_clipBeat == m_cachedCollisionsBeat && !m_cachedCollisionsNeedRefresh) {
            m_cachedCollisionsForClipBeat = m_cachedCollisionPairs;
        }
        else {
            // Require caches to update
            m_clipBeatCollisionsNeedRefresh = true;
            m_cachedCollisionsNeedRefresh = true;
        }
    }

    m_clipBeat = clipBeat;
}

const std::set<unsigned>& CollisionSpace::getMarchersAt(unsigned gridX, unsigned gridY, unsigned beat) const
{
    return m_marcherGrid.at(gridX).at(gridY).at(beat);
}

std::set<unsigned>& CollisionSpace::getMarchersAt(unsigned gridX, unsigned gridY, unsigned beat)
{
    return m_marcherGrid[gridX][gridY][beat];
}

void CollisionSpace::_forgetCollision(unsigned int firstMarcher, unsigned int secondMarcher, unsigned int beat)
{
    auto& collisionsBetweenThem = m_collisions[firstMarcher][secondMarcher];
    auto theCollision = collisionsBetweenThem.find(beat);
    if (theCollision != collisionsBetweenThem.end()) {
        collisionsBetweenThem.erase(theCollision);
    }
    if (firstMarcher < secondMarcher) {
        _forgetCollision(secondMarcher, firstMarcher, beat);
    }
}

void CollisionSpace::_registerCollision(unsigned int firstMarcher, unsigned int secondMarcher, unsigned int beat, SolverCoord pos)
{
    m_collisions[firstMarcher][secondMarcher][beat] = pos;
    if (firstMarcher < secondMarcher) {
        _registerCollision(secondMarcher, firstMarcher, beat, pos);
    }
}

void CollisionSpace::forgetCollision(unsigned int firstMarcher, unsigned int secondMarcher, unsigned int beat)
{
    if (firstMarcher < secondMarcher) {
        _forgetCollision(firstMarcher, secondMarcher, beat);
    }
    else {
        _forgetCollision(secondMarcher, firstMarcher, beat);
    }
}

void CollisionSpace::registerCollision(unsigned int firstMarcher, unsigned int secondMarcher, unsigned int beat, SolverCoord pos)
{
    if (firstMarcher < secondMarcher) {
        _registerCollision(firstMarcher, secondMarcher, beat, pos);
    }
    else {
        _registerCollision(secondMarcher, firstMarcher, beat, pos);
    }
}

void CollisionSpace::removeMarcher(unsigned marcher, unsigned gridX, unsigned gridY, unsigned beat, const SolverCoord& moveVectorFromPrevBeat)
{
    auto& marchersAtGridLoc = getMarchersAt(gridX, gridY, beat);

    marchersAtGridLoc.erase(marcher);
    for (auto otherMarcher : marchersAtGridLoc) {
        forgetCollision(marcher, otherMarcher, beat);
    }

    // Retract collisions resulting from swaps
    if (beat > 0 && !(moveVectorFromPrevBeat == 0)) {
        SolverCoord thisLocation((int32_t)gridX, (int32_t)gridY);
        auto swappingMarchers = getMarchersWithMovePattern(beat - 1, thisLocation, thisLocation - moveVectorFromPrevBeat);
        for (auto otherMarcher : swappingMarchers) {
            forgetCollision(marcher, otherMarcher, beat);
        }
    }
}

void CollisionSpace::placeMarcher(unsigned marcher, unsigned gridX, unsigned gridY, unsigned beat, const SolverCoord& moveVectorFromPrevBeat)
{
    SolverCoord thisLocation((int32_t)gridX, (int32_t)gridY);
    auto& marchersAtGridLoc = getMarchersAt(gridX, gridY, beat);
    for (auto otherMarcher : marchersAtGridLoc) {
        registerCollision(marcher, otherMarcher, beat, CalChart::Coord(gridX, gridY));
    }

    // Add collisions resulting from swaps
    if (beat > 0 && !(moveVectorFromPrevBeat == 0)) {
        SolverCoord thisLocation((int32_t)gridX, (int32_t)gridY);
        auto swappingMarchers = getMarchersWithMovePattern(beat - 1, thisLocation, thisLocation - moveVectorFromPrevBeat);
        for (auto otherMarcher : swappingMarchers) {
            registerCollision(marcher, otherMarcher, beat, /*CalChart::Coord(gridX, gridY)*/ CalChart::Coord(0, 0));
        }
    }

    marchersAtGridLoc.insert(marcher);
}

std::set<unsigned> CollisionSpace::getMarchersWithMovePattern(unsigned startBeat, SolverCoord firstCoord, SolverCoord secondCoord) const
{
    std::set<unsigned> result;

    auto& marchersAtFirstCoord = getMarchersAt(firstCoord.x, firstCoord.y, startBeat);
    auto& marchersAtSecondCoord = getMarchersAt(secondCoord.x, secondCoord.y, startBeat + 1);

    for (unsigned i : marchersAtFirstCoord) {
        if (marchersAtSecondCoord.find(i) != marchersAtSecondCoord.end()) {
            result.insert(i);
        }
    }

    return result;
}

/*!
 * @brief Converts all of the point positions in a CalChart stuntsheet into the SolverCoord
 * coordinate system, and pushes the converted positions into a provided vector.
 * @param sheet The CalChart stuntsheet whose dot positions will be converted.
 * @param positions A reference to the target vector that will receive all of the new
 * converted dot positions.
 */
void convertPositionsOnSheetToSolverSpace(const CalChart::Sheet& sheet, std::vector<SolverCoord>& positions)
{
    auto points = sheet.GetPoints();
    for (size_t i = 0; i < points.size(); i++) {
        positions.push_back(SolverCoord::fromShowSpace(points[i].GetPos()));
    }
}

/*!
 * @brief Creates a matrix of distances to be used with the Hungarian Algorithm,
 * for assigning marchers to destinations in such a way as to attempt to minimize
 * the total distance traveled by the marchers.
 * @detail In the returned matrix, the value at matrix(i,j) gives the manhattan
 * distance from the start location of the marcher at index i to the destination
 * at index j. The manhattan distance is equal to the number of steps that the
 * marcher would take to the destination, if it were following a NSEW or EWNS move
 * pattern. If the marcher cannot reach a particular destination within the number
 * of beats allowed for the transition, then the distance for that marcher to
 * that destination will be replaced by some huge number, effectively indicating
 * that the destination is impossible to reach.
 * @param startPositions A list of all of the marcher start locations, indexed
 * by the indices of the marchers that start at those locations.
 * @param endPositions A list of all of the possible destinations that the marchers
 * can reach.
 * @param numBeats The maximum number of beats available for the transition.
 * @return A matrix cataloguing the costs associated with moving each marcher
 * to each potential destination. The cost associated with moving marcher i to
 * destination j can be found at matrix position matrix(i,j).
 */
Matrix<double> makeHungarianDistanceMatrix(const std::vector<SolverCoord>& startPositions, const std::vector<SolverCoord>& endPositions, unsigned numBeats)
{
    size_t numMarchers = startPositions.size();
    Matrix<double> matrix = Matrix<double>(numMarchers, numMarchers);
    for (size_t i = 0; i < endPositions.size(); i++) {
        for (size_t j = 0; j < startPositions.size(); j++) {
            auto diff = startPositions.at(j) - endPositions.at(i);
            unsigned manhattanDist = abs(diff.x) + abs(diff.y);
            if (manhattanDist > numBeats) {
                matrix(j, i) = std::numeric_limits<double>::max(); //std::numeric_limits<double>::infinity();
            }
            else {
                matrix(j, i) = manhattanDist + 1;
            }
        }
    }
    return matrix;
}

/*!
 * @brief An enumeration containing all cardinal and intercardinal
 * directions.
 */
enum CompassDirection {
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

/*!
 * @brief An enumeration containing only the cardinal directions.
 */
enum CardinalCompassDirection {
    CARDINAL_COMPASS_N = COMPASS_N,
    CARDINAL_COMPASS_E = COMPASS_E,
    CARDINAL_COMPASS_S = COMPASS_S,
    CARDINAL_COMPASS_W = COMPASS_W,
    CARDINAL_COMPASS_INVALID = COMPASS_INVALID,
};

/*!
 * @brief The set of primitive vectors from which all other
 * step vectors are derived, using only an optional
 * negation operator.
 */
const SolverCoord kZeroVector(0, 0);
const SolverCoord kNVector(1, 0);
const SolverCoord kEVector(0, 1);
const SolverCoord kNEVector(1, 1);
const SolverCoord kSEVector(1, -1);

/*!
 * @brief A vector that can be used to convert from
 * compass directions to the step vectors oriented
 * toward those compass directions.
 * @detail kMoveVectorsByDirection[COMPASS_DIR] will
 * give the step vector oriented in the direction
 * of COMPASS_DIR.
 */
const SolverCoord kMoveVectorFromDirection[COMPASS_INVALID + 1] = {
    /* kMoveVectorsByDirection[COMPASS_N] = */ kNVector,
    /* kMoveVectorsByDirection[COMPASS_NE] = */ kNEVector,
    /* kMoveVectorsByDirection[COMPASS_E] = */ kEVector,
    /* kMoveVectorsByDirection[COMPASS_SE] = */ kSEVector,
    /* kMoveVectorsByDirection[COMPASS_S] = */ -kNVector,
    /* kMoveVectorsByDirection[COMPASS_SW] = */ -kNEVector,
    /* kMoveVectorsByDirection[COMPASS_W] = */ -kEVector,
    /* kMoveVectorsByDirection[COMPASS_NW] = */ -kSEVector
        /* kMoveVectorsByDirection[COMPASS_INVALID] = */
        - kZeroVector
};

/*!
 * @brief A two-dimentional array that can be used to translate a
 * movement vector into the compass direction for that vector.
 * @detail kDirectionByMoveVector[x][y] will give the compass
 * direction associated with a movement vector (x,y).
 */
const CompassDirection kDirectionFromMoveVector[3][3] = {
    { COMPASS_SW, COMPASS_S, COMPASS_SE },
    { COMPASS_W, COMPASS_INVALID, COMPASS_E },
    { COMPASS_NW, COMPASS_N, COMPASS_NE }
};

/*!
 * @brief Tracks the cardinal directions that must be travelled
 * in order to reach a particular destination from a start point.
 * @detail The axes are categorized as 'major' or 'minor'. The meaning
 * of 'major' and 'minor' in this context is tied to the distances
 * that the marcher would have to travel toward each of the
 * cardinal directions in order to reach the destination. The
 * cardinal direction toward which the marcher would have to move farther
 * is the 'major' axis; the other direction is the 'minor' axis.
 */
struct PathAxes {
    CardinalCompassDirection majorAxis = CARDINAL_COMPASS_INVALID;
    CardinalCompassDirection minorAxis = CARDINAL_COMPASS_INVALID;
};

/*!
 * @brief Track the solution for a particular marcher by associating
 * it with a destination and instruction for getting there.
 * @detail Note that theses solutions are not final until the solver
 * completes. Before then, the solutions cannot really be considered
 * solutions, because they will likely result in collisions or other
 * problems.
 */
struct MarcherSolution {
    /*!
     * @brief The position at which the marcher starts its transition.
     */
    SolverCoord startPos;

    /*!
     * @brief The position at which the marcher ends its transition.
     */
    SolverCoord endPos;

    /*!
     * @param The instruction for how the marcher should move from
     * its start location to its destination.
     */
    TransitionSolverParams::MarcherInstruction instruction;
};

/*!
 * @brief Calculates the cardinal directions toward which a marcher
 * would need to travel in order to reach the specified destination
 * from the specified start location.
 * @param startPos The start location for a marcher.
 * @param endPos The destination for a marcher.
 * @return The cardinal directions toward which a marcher would travel
 * to reach endPos from startPos.
 */
PathAxes calcPathAxes(SolverCoord startPos, SolverCoord endPos)
{
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
        result.majorAxis = (CardinalCompassDirection)kDirectionFromMoveVector[diff.x + 1][1];
        result.minorAxis = (CardinalCompassDirection)kDirectionFromMoveVector[1][diff.y + 1];
    }
    else {
        result.majorAxis = (CardinalCompassDirection)kDirectionFromMoveVector[1][diff.y + 1];
        result.minorAxis = (CardinalCompassDirection)kDirectionFromMoveVector[diff.x + 1][1];
    }
    return result;
}

/*!
 * @brief Calculates the step vectors that a marcher would use during its first and
 * second movement phase, given the cardinal directions of its movement and the
 * instruction that it must use to reach the end.
 * @param axes The cardinal directions along which the marcher could move to reach
 * its destination.
 * @param instruction The current instruction that the marcher must use to reach
 * its destination.
 * @return A pair of step vectors that a marcher would use to reach a destination
 * given its current instruction. The first element in the pair is the step
 * vector associated with the first movement phase, and the second element is
 * associated with the second movement phase.
 */
std::pair<SolverCoord, SolverCoord> calcStepVectors(PathAxes axes, const TransitionSolverParams::MarcherInstruction& instruction)
{
    std::pair<SolverCoord, SolverCoord> stepVectors;
    bool majorStepIsFirst = false;
    SolverCoord majorStep = kMoveVectorFromDirection[axes.majorAxis];
    SolverCoord minorStep = kMoveVectorFromDirection[axes.minorAxis];
    switch (instruction.movementPattern) {
    case TransitionSolverParams::MarcherInstruction::Pattern::EWNS:
        majorStepIsFirst = !(minorStep.y != 0);
        break;
    case TransitionSolverParams::MarcherInstruction::Pattern::NSEW:
        majorStepIsFirst = !(minorStep.x != 0);
        break;
    case TransitionSolverParams::MarcherInstruction::Pattern::HSDM:
        majorStepIsFirst = true;
    case TransitionSolverParams::MarcherInstruction::Pattern::DMHS:
        minorStep += majorStep;
        break;
    default:
        break;
    }
    if (majorStepIsFirst) {
        stepVectors.first = majorStep;
        stepVectors.second = minorStep;
    }
    else {
        stepVectors.first = minorStep;
        stepVectors.second = majorStep;
    }

    if (stepVectors.first == stepVectors.second) {
        stepVectors.second = 0;
    }

    return stepVectors;
}

/*!
 * @brief Given two step vectors, a start position, and a destination, this solves the number of steps
 * that a marcher must move along each of these step vectors in order to reach the destination
 * from the start position.
 * @param startPos The start position of the movement.
 * @param endPos The final destination of the movement.
 * @param stepVector1 A vector containing the number of x-units and y-units that are traversed on each
 * step of a marcher's first movement phase.
 * @param stepVector2 A vector containing the number of x-units and y-units that are traversed on each
 * step of a marcher's second movment phase.
 * @return A pair of floats indicating the number of steps that a marcher must move along each step
 * vector in order to reach the destination position from the start position. The first element
 * in the pair is the number of steps that should be moved along the first move vector; the second
 * element in the pair is the number of steps that should be moved along the second move vector.
 */
std::pair<float, float> calcStepsInEachDir(SolverCoord startPos, SolverCoord endPos, SolverCoord stepVector1, SolverCoord stepVector2)
{
    // Make sure that the first vector has an x-component, since we'll be dividing by it later
    // We know at least one of the vectors will have an x-component, since they must be orthogonal
    SolverCoord vec1, vec2;
    bool swap = (stepVector1.x == 0);
    if (swap) {
        vec1 = stepVector2;
        vec2 = stepVector1;
    }
    else {
        vec1 = stepVector1;
        vec2 = stepVector2;
    }

    // startPos + (numStepsLeg1 * stepVector1) + (numStepsLeg2 * stepVector2) = endPos
    // (numStepsLeg1 * stepVector1) + (numStepsLeg2 * stepVector2) = endPos - startPos = moveVector
    SolverCoord moveVector = endPos - startPos;

    float numSteps1;
    float numSteps2;

    if (vec1.x != 0) {
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
        if (vec2.x == 0 && vec2.y == 0) {
            numSteps2 = 0;
        }
        else {
            numSteps2 = (moveVector.y - (a * vec1.y)) / (vec2.y - (b * vec1.y));
        }

        numSteps1 = a - (b * numSteps2);
    }
    else {
        if (vec1.y != 0) {
            numSteps1 = moveVector.y / vec1.y;
            numSteps2 = 0;
        }
        else if (vec2.y != 0) {
            numSteps1 = 0;
            numSteps2 = moveVector.y / vec2.y;
        }
        else {
            numSteps1 = 0;
            numSteps2 = 0;
        }
    }

    if (swap) {
        return { numSteps2, numSteps1 };
    }
    else {
        return { numSteps1, numSteps2 };
    }
}

/*!
 * @brief Breaks down a marcher's current solution into the movement
 * that it represents.
 * @param marcher The current solution for a marcher.
 * @return The movement represented by the marcher's solution.
 */
MovingMarcher calculateMovementFromSolution(const MarcherSolution& marcher)
{
    PathAxes pathAxes = calcPathAxes(marcher.startPos, marcher.endPos);
    std::pair<SolverCoord, SolverCoord> stepVectors = calcStepVectors(pathAxes, marcher.instruction);
    std::pair<float, float> numSteps = calcStepsInEachDir(marcher.startPos, marcher.endPos, stepVectors.first, stepVectors.second);

    MovingMarcher marcherAnim;
    marcherAnim.waitBeats = marcher.instruction.waitBeats;
    marcherAnim.stepVectors = stepVectors;
    marcherAnim.numSteps = numSteps;
    marcherAnim.startPos = marcher.startPos;

    return marcherAnim;
}

class FileReader {
public:
    static std::vector<SolverCoord> readInitialFile(std::string filepath);
    static std::vector<SolverCoord> readFinalFile(std::string filepath);

private:
};

std::vector<SolverCoord> FileReader::readInitialFile(std::string filepath)
{
    std::map<unsigned, SolverCoord> positionsByIndex;
    std::vector<SolverCoord> positions;
    std::ifstream file(filepath);

    if (file.is_open()) {
        int32_t y = 0;
        uint32_t maxMarcherIndex = 0;
        std::string currLine;

        while (getline(file, currLine)) {
            int32_t x = 0;
            int32_t gridOccupant;
            std::istringstream lineStream(currLine);
            std::istream_iterator<std::string> wordsBegin(lineStream), wordsEnd;
            std::vector<std::string> rowOccupants(wordsBegin, wordsEnd);

            for (unsigned i = 0; i < rowOccupants.size(); i++) {
                gridOccupant = std::stoi(rowOccupants[i]);

                if (gridOccupant > 0) {
                    positionsByIndex[gridOccupant] = SolverCoord(x, y);

                    if ((uint32_t)gridOccupant > maxMarcherIndex) {
                        maxMarcherIndex = gridOccupant;
                    }
                }

                x += 2;
            }

            y += 2;
        }

        for (unsigned marcherIndex = 0; marcherIndex < maxMarcherIndex; marcherIndex++) {
            positions.push_back(positionsByIndex[marcherIndex + 1]);
        }
    }

    return positions;
}

std::vector<SolverCoord> FileReader::readFinalFile(std::string filepath)
{
    std::map<unsigned, SolverCoord> positionsByIndex;
    std::vector<SolverCoord> positions;
    std::string currLine;
    std::ifstream file(filepath);

    if (file.is_open()) {
        int32_t y = 0;
        std::string currLine;

        while (getline(file, currLine)) {
            int32_t x = 0;
            bool hasGridOccupant;
            std::istringstream lineStream(currLine);
            std::istream_iterator<std::string> wordsBegin(lineStream), wordsEnd;
            std::vector<std::string> rowOccupants(wordsBegin, wordsEnd);

            for (unsigned i = 0; i < rowOccupants.size(); i++) {
                hasGridOccupant = (std::stoi(rowOccupants[i]) > 0);

                if (hasGridOccupant) {
                    positions.push_back(SolverCoord(x, y));
                }

                x += 2;
            }

            y += 2;
        }
    }

    std::sort(positions.begin(), positions.end(),
        [](SolverCoord const& a, SolverCoord const& b) {
                  if (a.x < b.x) {
                      return true;
                  }
                  else if (a.x > b.x) {
                      return false;
                  }
                  else
                  {
                      return a.y < b.y;
                  } });

    return positions;
}

/*!
 * @brief Summarizes which matchings are allowed between marcher
 * and destination.
 */
class DestinationConstraints {
public:
    /*!
     * @brief Constructor.
     * @param groupConstraints A list of marcher groups that
     * have an explicit set of allowable destiations.
     * @param destinations A list of all of the destinations
     * in the transition.
     */
    DestinationConstraints(const std::vector<TransitionSolverParams::GroupConstraint>& groupConstraints, const std::vector<SolverCoord>& destinations);

    /*!
     * @brief Returns whether or not the marcher with the specified
     * index can be matched with the destination having the specified index.
     * @param marcher The index of the marcher that is being tested
     * against the destination.
     * @param destination The index of the destination to test against the
     * marcher.
     * @return True if the marcher is allowed to be matched with the provided
     * destination; false otherwise.
     */
    bool destinationIsAllowed(unsigned marcher, unsigned destination) const;

    /*!
     * @brief Returns whether or not the marcher with the specified
     * index can be matched with the specified destination coordinate.
     * @param marcher The marcher that is being tested against the
     * destination.
     * @param destination The destination coordinate to check against
     * the marcher.
     * @return True if the marcher is allowed to be matched with the
     * specified destination; false otherwise.
     */
    bool destinationIsAllowed(unsigned marcher, SolverCoord destination) const;

private:
    /*!
     * @brief A comparator used to control the internal structure
     * of m_destinationPositionsToIndices.
     */
    class SolverCoordCompare {
    public:
        bool operator()(const SolverCoord& first, const SolverCoord& second) const
        {
            bool result;
            if (first.x == second.x) {
                result = (first.y < second.y);
            }
            else {
                result = (first.x < second.x);
            }
            return result;
        }
    };

    std::map<unsigned, std::set<unsigned> > m_allowedDestinations;
    std::map<SolverCoord, unsigned, SolverCoordCompare> m_destinationPositionsToIndices;
};

DestinationConstraints::DestinationConstraints(const std::vector<TransitionSolverParams::GroupConstraint>& groupConstraints, const std::vector<SolverCoord>& destinations)
{
    for (TransitionSolverParams::GroupConstraint group : groupConstraints) {
        for (auto marcher : group.marchers) {
            for (auto destination : group.allowedDestinations) {
                m_allowedDestinations[marcher].insert(destination);
            }
        }
    }
    for (unsigned i = 0; i < destinations.size(); i++) {
        m_destinationPositionsToIndices[destinations[i]] = i;
    }
}

bool DestinationConstraints::destinationIsAllowed(unsigned marcher, unsigned destination) const
{
    bool result = true;

    if (m_allowedDestinations.find(marcher) != m_allowedDestinations.end()) {
        const std::set<unsigned>& allowedDestinations = m_allowedDestinations.at(marcher);

        result = allowedDestinations.find(destination) != allowedDestinations.end();
    }

    return result;
}

bool DestinationConstraints::destinationIsAllowed(unsigned marcher, SolverCoord destination) const
{
    return destinationIsAllowed(marcher, m_destinationPositionsToIndices.at(destination));
}

#pragma mark - Algorithm By: Chiu Zamora Malani

// ==============================================
// ===-- ALGORITHM BY: CHIU, ZAMORA, MALANI --===
// ==============================================
// Algorithm Outline:
// ---
// - Using the Hungarian Algorithm, come up with an initial set of destination assignments for the marchers
//   (arbitrarily assign paths for getting to those destinations)
// - Until a solution for the transition is found, do the following:
//     - Iterate over successively larger transition durations, starting from zero and ending at the maximum duration for
//       the transition; for each truncated duration, do the following:
//        - Identify all pairs of colliding marchers
//        - For each pair of colliding marchers:
//            - Attempt all options for solving the collision in isolation, which are:
//               (1) Swap the marchers' destinations
//               (2) Change the shapes of the paths that the marchers follow to reach their destinations
//               (3) Change the amount of time that the marchers wait before departing for their destinations
//            - Of all of the attempted options, select the one that overall solves the most collisions over the entire field
//            - Assign new instructions and destinations to the colliding marchers to reflect the selected option

namespace e7ChiuZamoraMalani {

    /*!
     * @brief Describes a potential option for solving a collision
     * between two marchers.
     * @discussion When considering a collision between two marchers
     * in isolation, you can solve the collision using some combination
     * of three actions. First, you can swap the destinations of the two
     * marchers. Second, you can change the paths that each marcher will
     * use to reach their destination. Third, you can change the amount
     * of time that each marcher waits before moving. The second and
     * third actions can be summarized by the instructions assigned to
     * each marcher.
     */
    struct SolutionAdjustmentInstruction {
        /*!
         * @brief The index of the instruction that will be assigned
         * to the first marcher involved in the collision.
         */
        unsigned instructionForMarcher1;

        /*!
         * @brief The index of the instruction that will be assigned
         * to the second marcher involved in the collision.
         */
        unsigned instructionForMarcher2;

        /*!
         * @brief True if the marchers' destinations should be swapped;
         * false otherwise.
         */
        bool swapMarchers;
    };

    /*!
     * @brief A comparator that sorts SolutionAdjustmentInstructions
     * such that, when solving a collision in isolation, the adjustments
     * that are listed earlier should be tried before those that are listed later.
     */
    struct AdjustmentInstructionSorter {

        AdjustmentInstructionSorter(const std::vector<TransitionSolverParams::MarcherInstruction>& instructionOptions)
            : instructionOptions(instructionOptions){};

        const std::vector<TransitionSolverParams::MarcherInstruction> instructionOptions;

        bool operator()(const SolutionAdjustmentInstruction& first, const SolutionAdjustmentInstruction& second)
        {
            unsigned waitBeats1;
            unsigned waitBeats2;
            unsigned marchPattern1;
            unsigned marchPattern2;

            waitBeats1 = instructionOptions.at(first.instructionForMarcher1).waitBeats + instructionOptions.at(first.instructionForMarcher2).waitBeats;
            waitBeats2 = instructionOptions.at(second.instructionForMarcher1).waitBeats + instructionOptions.at(second.instructionForMarcher2).waitBeats;

            marchPattern1 = (unsigned)instructionOptions[first.instructionForMarcher1].movementPattern + (unsigned)instructionOptions[first.instructionForMarcher2].movementPattern;
            marchPattern2 = (unsigned)instructionOptions[second.instructionForMarcher1].movementPattern + (unsigned)instructionOptions[second.instructionForMarcher2].movementPattern;

            if (waitBeats1 == waitBeats2) 
            {
                if (marchPattern1 == marchPattern2) 
                {
					if (first.swapMarchers == second.swapMarchers)
					{
						return false;
					}
					else
					{
						return !first.swapMarchers;
					}
                } 
                else
                {
                    return marchPattern1 < marchPattern2;
                }
            } 
            else 
            {
                return waitBeats1 < waitBeats2;
            }
        }
    };

    /*!
     * @brief Given a list of all of the instructions that can be applied
     * to a marcher, returns a list of all of the ways in which an isolated
     * collision can be solved.
     * @detail Note that the options are ordered, such that the preferred
     * adjustments come earlier in the list.
     * @param instructionOptions A list of the instructions that can be applied
     * to a marcher.
     * @return A list of all of the options for solving an isolated collision
     * between two marchers.
     */
    std::vector<SolutionAdjustmentInstruction> unfilteredAdjustmentOptions(const std::vector<TransitionSolverParams::MarcherInstruction>& instructionOptions)
    {
        std::vector<SolutionAdjustmentInstruction> allOptions;
        for (unsigned inst1 = 0; inst1 < instructionOptions.size(); inst1++) {
            for (unsigned inst2 = 0; inst2 < instructionOptions.size(); inst2++) {
                for (bool swapPositions : { false, true }) {
                    allOptions.push_back({ inst1, inst2, swapPositions });
                }
            }
        }
        std::sort(allOptions.begin(), allOptions.end(), AdjustmentInstructionSorter(instructionOptions));
        return allOptions;
    }

    /*!
     * @brief Assigns a new destination and instruction to a marcher,
     * updating both its movement command and its solution in the
     * process.
     * @param marcher The marcher's current movement command, which will
     * be updated as the new destination and instruction are applied.
     * @param marcherSolution The marcher's current solution, which
     * will be updated as the new destination and instruction are applied
     * @param instruction The new instruction to apply to the marcher.
     * @parma newDestination The new destination to assign to the marcher.
     */
    void recalculateMarcher(MovingMarcher& marcher, MarcherSolution& marcherSolution, const TransitionSolverParams::MarcherInstruction& instruction, SolverCoord newDestination)
    {
        marcherSolution.endPos = newDestination;
        marcherSolution.instruction = instruction;
        marcher = calculateMovementFromSolution(marcherSolution);
    }

    void iterateSolution(std::vector<MarcherSolution>& marcherSolutions, CollisionSpace& collisionSpace, unsigned maxBeats, const DestinationConstraints& destinationConstraints, const std::vector<TransitionSolverParams::MarcherInstruction>& instructionOptions, TransitionSolverDelegate* delegate)
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
            if (delegate) {
                double progress;

                if (originalNumCollisions > collisionSpace.collectCollisionPairs().size()) {
                    progress = (double)(originalNumCollisions - collisionSpace.collectCollisionPairs().size()) / (double)originalNumCollisions;
                }
                else {
                    progress = 1;
                }

                delegate->OnSubtaskProgress(progress);

                if (delegate->ShouldAbortCalculation()) {
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
                    unsigned activeOptionIndex = allActiveOptions[{ col.firstMarcher, col.secondMarcher }];
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
                        if (newOption.swapMarchers != activeOption.swapMarchers) {
                            SolverCoord tmp = destination1;
                            destination1 = destination2;
                            destination2 = tmp;
                        }

                        recalculateMarcher(marcherMove1, mutableMarcherSolution1, instructionOptions[newOption.instructionForMarcher1], destination1);
                        recalculateMarcher(marcherMove2, mutableMarcherSolution2, instructionOptions[newOption.instructionForMarcher2], destination2);
                        badSolutionDuration = (marcherMove1.waitBeats + marcherMove1.numSteps.first + marcherMove1.numSteps.second > maxBeats) || (marcherMove2.waitBeats + marcherMove2.numSteps.first + marcherMove2.numSteps.second > maxBeats);
                        badSolutionDestination = !destinationConstraints.destinationIsAllowed(col.firstMarcher, mutableMarcherSolution1.endPos) || !destinationConstraints.destinationIsAllowed(col.secondMarcher, mutableMarcherSolution2.endPos);

                        activeOption = newOption;
                        if ((badSolutionDuration || badSolutionDestination) && newOptionIndex != activeOptionIndex) {
                            continue;
                        }

                        collisionSpace.reinstructMarcher(col.firstMarcher, marcherMove1);
                        collisionSpace.reinstructMarcher(col.secondMarcher, marcherMove2);

                        allActiveOptions[{ col.firstMarcher, col.secondMarcher }] = newOptionIndex;
                        marcherSolutions[col.firstMarcher] = mutableMarcherSolution1;
                        marcherSolutions[col.secondMarcher] = mutableMarcherSolution2;

                        // If the collision was solved, go ahead and break from this
                        unsigned beatsWithoutCollision = collisionSpace.beatsBeforeCollisionBetweenMarchers(col.firstMarcher, col.secondMarcher);
                        if (beatsWithoutCollision > beat) {
                            progress = true;
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
            }
            else {
                numIterationsWithoutOverallImprovement++;
            }
        }
    }
}

#pragma mark - Algorithm By: Namini Asl, Ramirez, Zhang

// ===================================================
// ===-- ALGORITHM BY: NAMINI ASL, RAMIREZ, ZHANG --===
// ===================================================
// Algorithm Outline:
// ---
// - Using the Hungarian Algorithm, come up with an initial set of destination assignments for the marchers
//   (arbitrarily assign paths for getting to those destinations)
// - Iterate over successively larger transition durations, starting from zero and ending at the maximum duration for
//   the transition; for each truncated duration, do the following:
//    - Repeat the following steps a fixed number of times, in an attempt to solve all collisions occuring up to and including
//       this beat
//        - Identify all pairs of colliding marchers
//        - For each pair of colliding marchers:
//            - Attempt all options for solving the collision in isolation, which are:
//               (1) Swap the marchers' destinations
//               (2) Change the shapes of the paths that the marchers follow to reach their destinations
//               (3) Change the amount of time that the marchers wait before departing for their destinations
//            - Select the first option that is found to solve the collision between the two marchers (ignoring any collisions
//               produced for other marchers)
//            - Assign new instructions and destinations to the colliding marchers to reflect the selected option

namespace e7NaminiaslRamirezZhang {
    // Very similar to ChiuZamoraMalani
    // For each beat, try to solve all collisions by looking at each marcher and trading out options
    // However, the difference is: We try to get rid of ALL collisions on any given beat, instead of looping back to the beginning at the end (we have a max number of times we do this)
    // Second, we look through ALL options for each marcher, and choose the one that offers the LEAST number of collisions
    using namespace e7ChiuZamoraMalani;

    void iterateSolution(std::vector<MarcherSolution>& marcherSolutions, CollisionSpace& collisionSpace, unsigned maxBeats, const DestinationConstraints& destinationConstraints, const std::vector<TransitionSolverParams::MarcherInstruction>& instructionOptions, TransitionSolverDelegate* delegate)
    {
        const std::vector<SolutionAdjustmentInstruction> unfilteredOptions = unfilteredAdjustmentOptions(instructionOptions);

        std::vector<Collision> collisionPairs;
        std::vector<Collision> lastCollisionPairs;
        std::map<std::pair<unsigned, unsigned>, unsigned> allActiveOptions;

        for (unsigned beat = 1; beat <= maxBeats; beat += std::min((unsigned)1, maxBeats - beat + 1)) {

            if (delegate) {
                delegate->OnSubtaskProgress((double)beat / (double)maxBeats);
                if (delegate->ShouldAbortCalculation()) {
                    break;
                }
            }

            collisionSpace.clipToBeat(beat);

            for (unsigned improvementIteration = 0; improvementIteration < 3 || beat == maxBeats; improvementIteration++) {
                collisionPairs = collisionSpace.collectCollisionPairs();

                if (collisionPairs == lastCollisionPairs) {

                    break;
                }

                lastCollisionPairs = collisionSpace.collectCollisionPairs();

                std::random_shuffle(collisionPairs.begin(), collisionPairs.end());
                for (unsigned collisionIndex = 0; collisionIndex < collisionPairs.size(); collisionIndex++) {

                    Collision& col = collisionPairs[collisionIndex];

                    // For each pair of colliding marchers, select the appropriate set of options for solving the collision
                    const std::vector<SolutionAdjustmentInstruction>* adjustmentOptionsPtr;
                    adjustmentOptionsPtr = &unfilteredOptions;
                    const std::vector<SolutionAdjustmentInstruction>& adjustmentOptions = *adjustmentOptionsPtr;

                    // Find the most recent option that was used to fix the collision; start from there when figuring out what to do next with them
                    unsigned activeOptionIndex = allActiveOptions[{ col.firstMarcher, col.secondMarcher }];
                    SolutionAdjustmentInstruction activeOption = adjustmentOptions.at(activeOptionIndex);
                    unsigned bestOptionIndex = activeOptionIndex;
                    unsigned bestNumCollisions = (unsigned)collisionSpace.collectCollisionPairs().size();

                    // Check through all fix options, and find the one which makes the most improvement
                    MarcherSolution mutableMarcherSolution1 = marcherSolutions[col.firstMarcher];
                    MarcherSolution mutableMarcherSolution2 = marcherSolutions[col.secondMarcher];
                    for (unsigned newOptionIndex = 0;; newOptionIndex++) {
                        bool executingFinalIteration;

                        executingFinalIteration = (newOptionIndex == adjustmentOptions.size());
                        if (executingFinalIteration) {
                            newOptionIndex = bestOptionIndex;
                        }

                        SolutionAdjustmentInstruction newOption = adjustmentOptions.at(newOptionIndex);

                        MovingMarcher marcherMove1 = collisionSpace.getMarcherInstruction(col.firstMarcher);
                        MovingMarcher marcherMove2 = collisionSpace.getMarcherInstruction(col.secondMarcher);

                        bool badSolutionDuration = false;
                        bool badSolutionDestination = false;

                        SolverCoord destination1 = mutableMarcherSolution1.endPos;
                        SolverCoord destination2 = mutableMarcherSolution2.endPos;
                        if (newOption.swapMarchers != activeOption.swapMarchers) {
                            SolverCoord tmp = destination1;
                            destination1 = destination2;
                            destination2 = tmp;
                        }

                        recalculateMarcher(marcherMove1, mutableMarcherSolution1, instructionOptions[newOption.instructionForMarcher1], destination1);
                        recalculateMarcher(marcherMove2, mutableMarcherSolution2, instructionOptions[newOption.instructionForMarcher2], destination2);
                        badSolutionDuration = (marcherMove1.waitBeats + marcherMove1.numSteps.first + marcherMove1.numSteps.second > maxBeats) || (marcherMove2.waitBeats + marcherMove2.numSteps.first + marcherMove2.numSteps.second > maxBeats);
                        badSolutionDestination = !destinationConstraints.destinationIsAllowed(col.firstMarcher, mutableMarcherSolution1.endPos) || !destinationConstraints.destinationIsAllowed(col.secondMarcher, mutableMarcherSolution2.endPos);

                        activeOption = newOption;
                        if ((badSolutionDuration || badSolutionDestination) && newOptionIndex != activeOptionIndex) {
                            continue;
                        }

                        collisionSpace.reinstructMarcher(col.firstMarcher, marcherMove1);
                        collisionSpace.reinstructMarcher(col.secondMarcher, marcherMove2);

                        allActiveOptions[{ col.firstMarcher, col.secondMarcher }] = newOptionIndex;
                        marcherSolutions[col.firstMarcher] = mutableMarcherSolution1;
                        marcherSolutions[col.secondMarcher] = mutableMarcherSolution2;

                        if (executingFinalIteration) {
                            break;
                        }

                        // Track our improvement
                        if (collisionSpace.collectCollisionPairs().size() <= bestNumCollisions) {
                            bestNumCollisions = (unsigned)collisionSpace.collectCollisionPairs().size();
                            bestOptionIndex = newOptionIndex;
                        }
                    }
                }
            }
        }
    };
}

#pragma mark - Algorithm By: Sover, Eliceiri, Hershkovitz

// ======================================================
// ===-- ALGORITHM BY: SOVER, ELICEIRI, HERSHKOVITZ --===
// ======================================================
// Algorithm Outline:
// ---
// - Using the Hungarian Algorithm, come up with an initial set of destination assignments for the marchers
//   (arbitrarily assign paths for getting to those destinations)
// - Arbitrarily decide an initial 'march order' for the marchers; this is the order in which we will place the
//   marchers onto the field
// - Until we've found a solution:
//    - Wipe the field clean, so that there are no marchers on it
//    - Iterate over each of the marchers, in the order dictated by the current 'march order'; for each of them,
//      do the following:
//       - For the current marcher, iterate over all paths that can be used to bring the marcher to its destination
//         (while altering the path, take care to prioritize altering the shape of the path over altering the number
//         of beats that the marcher will initially wait before moving)
//          - Try to place that marcher onto the field using the current path
//          - If the marcher can be placed without causing any collisions, then move onto the next marcher
//          - Otherwise, keep searching through the possiple paths
//       - If we can iterate over all potential paths without finding one that we can assign to the marcher without
//         causing collisions, then keep the marcher off the field and add it to a list of unplaced marchers
//    - Now that we've iterated over all of our marchers, check to see if we successfully placed them all
//       - If all of the marchers are placed, then we're done
//       - Otherwise, if we have marchers in our list of unplaced marchers, then we reorganize the march order so that
//          those unplaced marchers are moved to the front (the remaining marchers are kept in the same march order, relative
//          to one another, as before)

namespace e7SoverEliceiriHershkovitz {

    void iterateSolution(std::vector<MarcherSolution>& marcherSolutions, CollisionSpace& collisionSpace, unsigned maxBeats, const DestinationConstraints& destinationConstraints, const std::vector<TransitionSolverParams::MarcherInstruction>& instructionOptions, TransitionSolverDelegate* delegate)
    {
        // Prioritize commands in the order that we will be willing to give them to a marcher
        // We're much more willing to change direction than to increase the number of wait beats
        std::vector<TransitionSolverParams::MarcherInstruction> prioritizedInstructionOptions = instructionOptions;
        auto comparator = [](const TransitionSolverParams::MarcherInstruction& first, const TransitionSolverParams::MarcherInstruction& second) {
            if (first.waitBeats == second.waitBeats) {
                return first.movementPattern < second.movementPattern;
            }
            else {
                return first.waitBeats < second.waitBeats;
            }
        };
        std::sort(prioritizedInstructionOptions.begin(), prioritizedInstructionOptions.end(), comparator);

        // Create a mapping of all of the marchers to their current instructions
        std::vector<unsigned> activeInstructions;
        for (unsigned i = 0; i < marcherSolutions.size(); i++) {
            activeInstructions.push_back(0);
        }

        // Create an initial order for setting down our marchers
        std::vector<unsigned> marchOrder;
        for (unsigned i = 0; i < marcherSolutions.size(); i++) {
            marchOrder.push_back(i);
        }

        std::vector<unsigned> unplacedMarchers = marchOrder;
        const unsigned maxIterations = 250;
        unsigned bestNumberOfUnplacedMarchers = (unsigned)marcherSolutions.size();
        unsigned numIterationsWithoutImprovement = 0;
        for (unsigned numIterations = 0; numIterations < maxIterations && !collisionSpace.isSolved(); numIterations++) {
            if (delegate) {
                delegate->OnSubtaskProgress(std::max((double)(marchOrder.size() - unplacedMarchers.size()) / (double)marchOrder.size(), (double)numIterations / (double)maxIterations));
                if (delegate->ShouldAbortCalculation()) {
                    break;
                }
            }

            // Reset all of the marchers -- we will place them back one-by-one
            for (unsigned i = 0; i < marcherSolutions.size(); i++) {
                collisionSpace.disableMarcher(i);
            }

            // Place marchers onto the field in the march order
            unplacedMarchers.clear();
            for (unsigned marcher : marchOrder) {
                // Re-enable the marcher
                collisionSpace.enableMarcher(marcher);

                // Reset the marcher instruction back to one with the lowest wait time
                for (unsigned i = 0; i < prioritizedInstructionOptions.size(); i++) {
                    if (prioritizedInstructionOptions[activeInstructions[marcher]].movementPattern == prioritizedInstructionOptions[i].movementPattern) {
                        activeInstructions[marcher] = i;
                        break;
                    }
                }

                unsigned triedInstructions;
                for (triedInstructions = 0; triedInstructions < prioritizedInstructionOptions.size(); triedInstructions++) {
                    MovingMarcher newMoveInstructions;
                    e7ChiuZamoraMalani::recalculateMarcher(newMoveInstructions, marcherSolutions[marcher], prioritizedInstructionOptions[activeInstructions[marcher]], marcherSolutions[marcher].endPos);

                    if (collisionSpace.collectCollisionPairs().size() == 0 && collisionSpace.getMarchersWithIncompleteTransitions().size() == 0) {
                        break;
                    }

                    if (++activeInstructions[marcher] >= prioritizedInstructionOptions.size()) {
                        activeInstructions[marcher] = 0;
                    }
                }

                if (triedInstructions == prioritizedInstructionOptions.size()) {
                    unplacedMarchers.push_back(marcher);
                    collisionSpace.disableMarcher(marcher);
                }
            }

            // Remove all of the marchers that need to be re-prioritized
            {
                auto unplacedIter = unplacedMarchers.begin();
                for (auto marchersIter = marchOrder.begin(); marchersIter != marchOrder.end() && unplacedIter != unplacedMarchers.end(); marchersIter++) {
                    if (*marchersIter == *unplacedIter) {
                        marchOrder.erase(marchersIter--);
                        unplacedIter++;
                    }
                }
            }

            // Shuffle the people who need priority of replacement
            std::random_shuffle(unplacedMarchers.begin(), unplacedMarchers.end());

            // Re-add the unplaced marchers with higher priority
            marchOrder.insert(marchOrder.begin(), unplacedMarchers.begin(), unplacedMarchers.end());

            // Track our progress; we'll break if we're not making any
            numIterationsWithoutImprovement++;
            if (bestNumberOfUnplacedMarchers > unplacedMarchers.size()) {
                bestNumberOfUnplacedMarchers = (unsigned)unplacedMarchers.size();
                numIterationsWithoutImprovement = 0;
            }
            if (numIterationsWithoutImprovement >= 10) {
                break;
            }
        }
    };
}

#pragma mark - Final Solver

// ========================
// ===-- FINAL SOLVER --===
// ========================

std::vector<std::string> validateSheetForTransitionSolver(const CalChart::Sheet& sheet)
{
    std::vector<std::string> errors;

    // Verify that all points are in the grid, and that all of them can be converted to and from solver space
    for (unsigned i = 0; i < sheet.GetPoints().size(); i++) {
        CalChart::Coord showPosition;
        SolverCoord solverPosition;

        showPosition = sheet.GetPoint(i).GetPos();
        solverPosition = SolverCoord::fromShowSpace(showPosition);

        if (solverPosition.x % 2 != 0 || solverPosition.y % 2 != 0) {
            errors.push_back("Marcher " + std::to_string(i) + " is not located on a 2-step grid.");
        }
        if (solverPosition.toShowSpace() != showPosition) {
            errors.push_back("Bad location for marcher " + std::to_string(i));
        }
    }

    return errors;
}

TransitionSolverResult runSolverWithExplicitBeatCap(const CalChart::Sheet& sheet1, const CalChart::Sheet& sheet2, TransitionSolverParams params, unsigned numBeats, TransitionSolverDelegate* delegate)
{

    TransitionSolverResult results;

    // Convert the start and end locations of the stuntsheets so that they are represented in the SolverCoord coordinate system
    std::vector<SolverCoord> startPositions;
    std::vector<SolverCoord> endPositions;
    convertPositionsOnSheetToSolverSpace(sheet1, startPositions);
    convertPositionsOnSheetToSolverSpace(sheet2, endPositions);

    auto fieldWidth = SolverCoord::kFieldWidthInSteps;
    auto fieldHeight = SolverCoord::kFieldHeightInSteps;
    unsigned maxBeats = numBeats;

    // Scale the field and the transition duration by half, so that we can perform less calculations
    fieldWidth /= 2;
    fieldHeight /= 2;
    maxBeats /= 2;
    for (unsigned i = 0; i < startPositions.size(); i++) {
        startPositions[i] /= 2;
        endPositions[i] /= 2;
    }

    // Generate a preliminary cost matrix for the hungarian algorithm
    // This will minimize the distance travelled by our marchers (assuming they are moving either EWNS or NSEW) to get to the next stuntsheet, while assigning only one marcher to each destination
    // We will use this to assign reasonable destinations to each marcher to start
    Matrix<double> distances;
    DestinationConstraints destinationConstraints(params.groups, endPositions);
    distances = makeHungarianDistanceMatrix(startPositions, endPositions, maxBeats);

    // Here, we take into account the constraints that were passed to us in the TransitionSolverParams
    // For any assignment that matches a marcher to a destination that is not listed as one of its allowed destinations in the group constraints, we'll make the cost outrageously large
    for (unsigned i = 0; i < startPositions.size(); i++) {
        for (unsigned k = 0; k < endPositions.size(); k++) {
            if (!destinationConstraints.destinationIsAllowed(i, k)) {
                distances(i, k) = std::numeric_limits<double>::max();
            }
        }
    }

    // Use the hungarian algorithm to come up with a reasonable destination for each marcher (ignoring collisions)
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

    std::vector<TransitionSolverParams::MarcherInstruction> instructionOptions;
    for (unsigned i = 0; i < params.availableInstructions.size(); i++) {
        if (params.availableInstructionsMask[i]) {
            instructionOptions.push_back(params.availableInstructions[i]);
            instructionOptions[i].waitBeats /= 2;
        }
    }

    for (size_t i = 0; i < assignments.size(); i++) {
        marcherSolutions[i].startPos = startPositions[i];
        marcherSolutions[i].endPos = endPositions[assignments[i]];
        marcherSolutions[i].instruction = instructionOptions[0];

        MovingMarcher marcherAnim = calculateMovementFromSolution(marcherSolutions[i]);
        collisionSpace.reinstructMarcher((unsigned)i, marcherAnim);
    }

    if (delegate) {
        delegate->OnSubtaskProgress(0);
    }
    switch (params.algorithm) {
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
    if (delegate) {
        delegate->OnSubtaskProgress(1);
    }

    // Indicate the quality of the solution (or lack thereof) that we found
    results.successfullySolved = collisionSpace.isSolved();
    results.numBeatsOfMovement = collisionSpace.firstBeatAfterMovment() * 2;

    // Assign a final position to each marcher
    for (size_t i = 0; i < assignments.size(); i++) {
        results.finalPositions.push_back(SolverCoord::toShowSpace(marcherSolutions[i].endPos * 2)); // Make sure to scale the solution back up
    }

    // Assign a dot type for each marcher instruction that was allowed for the Transition Solver
    // For each of those dot types, generate the continuity text that represents the associated instruction
    std::map<std::pair<TransitionSolverParams::MarcherInstruction::Pattern, unsigned>, SYMBOL_TYPE> instructionToDotType;
    for (unsigned i = 0; i < instructionOptions.size(); i++) {
        SYMBOL_TYPE dotType = (SYMBOL_TYPE)i;
        std::string instructionString;
        const TransitionSolverParams::MarcherInstruction& instruction = instructionOptions.at(i);
        ;

        instructionToDotType[std::make_pair(instruction.movementPattern, instruction.waitBeats)] = dotType;

        switch (instruction.movementPattern) {
        case TransitionSolverParams::MarcherInstruction::Pattern::EWNS:
            instructionString = "ewns";
            break;
        case TransitionSolverParams::MarcherInstruction::Pattern::NSEW:
            instructionString = "nsew";
            break;
        case TransitionSolverParams::MarcherInstruction::Pattern::DMHS:
            instructionString = "dmhs";
            break;
        case TransitionSolverParams::MarcherInstruction::Pattern::HSDM:
            instructionString = "hsdm";
            break;
        default:
            break;
        }

        results.continuities[dotType] = "mt " + std::to_string(instruction.waitBeats * 2) + " e" + "\n" + instructionString + " np\n" + "mtrm e";
    }

    // Assign a dot type to each of the marchers that corresponds to the instruction that it was given
    for (unsigned i = 0; i < marcherSolutions.size(); i++) {
        const MarcherSolution& solution = marcherSolutions[i];

        results.marcherDotTypes.push_back(instructionToDotType[std::make_pair(solution.instruction.movementPattern, solution.instruction.waitBeats)]);
    }

    return results;
}

TransitionSolverResult runTransitionSolver(const CalChart::Sheet& sheet1, const CalChart::Sheet& sheet2, TransitionSolverParams params, TransitionSolverDelegate* delegate)
{

    TransitionSolverResult finalResult;
    TransitionSolverResult recentResult;
    unsigned scaledHighestBeatCap;
    unsigned scaledBeatCapForBestSolution;
    unsigned scaledBeatCapForCurrentCalculation;

    scaledBeatCapForBestSolution = 0;
    scaledHighestBeatCap = (sheet1.GetBeats() + (sheet1.GetBeats() / 2)) / 2;
    scaledBeatCapForCurrentCalculation = 0;
    finalResult.successfullySolved = false;
    finalResult.numBeatsOfMovement = sheet1.GetBeats();

    // Try to solve the transition at different transition durations, starting from 0 and increasing to from there
    // This will give us chances to find different solutions to the problem, and we can choose the best solution from the options
    // Since the best options are more likely to be found when we force a shorter number of beats in the transition, we'll start the transition duration at zero and count up
    while (scaledBeatCapForCurrentCalculation <= scaledHighestBeatCap) {

        // If a delegate was provided, indicate the current process, and offer an opportunity to end the calculation prematurely
        if (delegate) {
            delegate->OnProgress(((double)scaledBeatCapForCurrentCalculation) / (double)scaledHighestBeatCap);
            if (delegate->ShouldAbortCalculation()) {
                break;
            }
        }

        // Solve the transition for the current transition duration
        recentResult = runSolverWithExplicitBeatCap(sheet1, sheet2, params, scaledBeatCapForCurrentCalculation * 2, delegate);

        // If a solution was successfully found, compare it to our current favorite solution, and remember the new one instead if it is better
        if (recentResult.successfullySolved) {
            if (recentResult.numBeatsOfMovement <= finalResult.numBeatsOfMovement) {
                scaledBeatCapForBestSolution = scaledBeatCapForCurrentCalculation;
                finalResult = recentResult;
                if (delegate) {
                    delegate->OnNewPreferredSolution(finalResult.numBeatsOfMovement);
                }
            }
        }

        scaledBeatCapForCurrentCalculation += 1;
    }

    // Inform the delegate of the final solution, if a delegate exists
    if (delegate) {
        delegate->OnCalculationComplete(finalResult);
    }
    return finalResult;
}
}
