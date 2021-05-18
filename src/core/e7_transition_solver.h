#pragma once
//
//  e7_transition_solver.hpp
//  CalChart
//
//  Created by Kevin Durand on 10/12/16.
//
//

#include <array>
#include <vector>

#include "CalChartCoord.h"
#include "CalChartSheet.h"

namespace CalChart {

/*!
 * @brief Packages together all of the arguments
 * that are used by the transition solve.
 */
struct TransitionSolverParams {

    /*!
     * @brief An enumeration that uniquely identifies
     * each algorithm that can be used to solve a transition.
     */
    enum AlgorithmIdentifier {
        BEGIN = 0,
        E7_ALGORITHM__CHIU_ZAMORA_MALANI = BEGIN,
        E7_ALGORITHM__NAMINIASL_RAMIREZ_ZHANG,
        E7_ALGORITHM__SOVER_ELICEIRI_HERSHKOVITZ,
        END,
    };

    /*!
     * @brief Identifies a list of destinations that
     * are allowed for members of a particular marcher
     * group.
     * @detail Note that if a marcher is part of more than
     * one group, then that marcher will be allowed
     * to move to any destination for EITHER group.
     */
    struct GroupConstraint {
        GroupConstraint(){};

        /*!
         * @brief A collection of the indices of the marchers
         * that are included in this group.
         */
        std::set<unsigned> marchers;

        /*!
         * @brief A collection of the indices of the destinations
         * where the marchers in this group can finish.
         */
        std::set<unsigned> allowedDestinations;
    };

    /*!
     * @brief An instruction that can be assigned to a marcher.
     * @detail An instruction has a direct mapping to a CalChart continuity.
     * Each instruction consists of a certain number of mark time beats,
     * followed by a movement pattern that describes how to move to the next point.
     */
    struct MarcherInstruction {
        /*!
         * @brief The movement patterns that describe how a marcher
         * should move from its initial point to its final point.
         */
        enum Pattern {
            BEGIN = 0,
            EWNS = BEGIN,
            NSEW,
            DMHS,
            HSDM,
            END
        };

        MarcherInstruction(Pattern p = EWNS, unsigned beats = 0)
            : movementPattern(p)
            , waitBeats(beats){};

        /*!
         * @brief The movement pattern that will be assigned to any
         * marcher that follows this instruction.
         */
        Pattern movementPattern;

        /*!
         * @brief For any marcher following this instruction, the number
         * of beats to wait before moving.
         */
        unsigned waitBeats;
    };

    TransitionSolverParams()
    {
        for (size_t i = 0; i < availableInstructionsMask.size(); i++) {
            availableInstructionsMask[i] = false;
        }
    };

    /*!
     * @brief Identifies the algorithm that should be used to solve the transition.
     */
    AlgorithmIdentifier algorithm;

    /*!
     * @brief Identifies groups of marchers that have special rules about which
     * destinations they can be assigned.
     */
    std::vector<GroupConstraint> groups;

    /*!
     * @brief Identifies the instructions that can be assigned to the marchers
     * that are engaging in this transition.
     * @detail There can be up to eight instructions, since that is the maximum
     * number of dot types allowed by CalChart. Not all of the instructions need
     * to be used. Each of the instructions can be enabled/disabled using the
     * availableInstructionsMask. At each index of availableInstructions, the
     * instruction at that index is considered to be a valid instruction if
     * the value at the same index in availableInstructionsMask is true; otherwise,
     * the instruction is considered invalid.
     */
    std::array<MarcherInstruction, 8> availableInstructions;

    /*!
     * @brief Defines which instructions of the availableInstructions
     * can be used to solve the transition.
     * @detail For any given index, a value of true indicates that the
     * instruction at the corresponding index in availableInstructions
     * can be used to solve the transition. A value of false, conversely,
     * indicates that the instruction cannot be used.
     */
    std::array<bool, 8> availableInstructionsMask;
};

/*!
 * @brief A transition solution, as returned by the transition solver.
 */
struct TransitionSolverResult {

    /*!
     * @brief Identifies whether or not a solution was found.
     * @detail If this value is true, then a solution was found
     * successfully. If false, then no solution was found, and the
     * values for every other field in this struct are undefined.
     * Make sure to check the value of this field before using any
     * other fields.
     */
    bool successfullySolved;

    /*!
     * @brief If a solution was found, then this will contain
     * the number of beats that marchers actually move before stopping
     * in the solution.
     * @detail Note that if successfullySolved is false, the value of
     * this field is undefined.
     */
    unsigned numBeatsOfMovement;

    /*!
     * @brief If a solution was found, this will contain the destination
     * positions, indexed according to the marcher that should travel
     * to that destination.
     * @detail Note that if successfullySolved is false, the value
     * for this field is undefined.
     */
    std::vector<CalChart::Coord> finalPositions;

    /*!
     * @brief If a solution was found, this will contain the continuity
     * text for each dot type that is included in the solution.
     * @detail Note that if successfullySolved is false, the value for
     * this field is undefined.
     */
    std::map<SYMBOL_TYPE, std::string> continuities;

    /*!
     * @brief If a solution was found, this will contain the dot types
     * that should be assigned to each marcher, indexed by the marcher
     * indices associated with each dot type assignment.
     * @detail Note that if successfullySolved is false, the value
     * for this field is undefined.
     */
    std::vector<SYMBOL_TYPE> marcherDotTypes;
};

/*!
 * @brief A virtual base class for any object that wishes to engage
 * in the transition solving process by helping control when to abort
 * and by receiving live notifications about the progress of the solver while
 * it works.
 */
class TransitionSolverDelegate {
public:
    /*!
     * @brief This method will be called when the transition solver has
     * a new estimate for its overall progress.
     * @param progress A double in the range of 0 to 1 (inclusive) that
     * indicates the overall progress of the transition as a percentage.
     */
    virtual void OnProgress(double progress) = 0;

    /*!
     * @brief This method will be called when the transition solver
     * has a new estimate for the progress of the current subtask.
     * @param progress A double in the range of 0 to 1 (inclusive) that
     * indicates the progress of the current subtask as a percentage.
     */
    virtual void OnSubtaskProgress(double progress) = 0;

    /*!
     * @brief This method will be called when the transition solver
     * discovers a new, preferred solution.
     * @param numBeatsInSolution The number of beats of movement before
     * all marchers reach their destinations in the solution.
     */
    virtual void OnNewPreferredSolution(unsigned numBeatsInSolution) = 0;

    /*!
     * @brief This method will be called when the final solution
     * for the transition is selected.
     * @param finalSolution The final solution for the transition.
     */
    virtual void OnCalculationComplete(TransitionSolverResult finalSolution) = 0;

    /*!
     * @brief This method will be called when the transition solver has
     * reached a point from which it can potentially abort.
     * @return True if the solver should abort its calculation now and
     * select a final solution; false if the solver should continue searching for a
     * new, better solution.
     */
    virtual bool ShouldAbortCalculation() = 0;
};

/*!
 * @brief Checks whether or not a stuntsheet can be used as an input to the transition solver.
 * @param sheet The stuntsheet to validate.
 * @return A list of error strings indicating reasons why this stuntsheet cannot be used
 * as an input to the transition solver. If the error list is empty, there were no issues,
 * and the stuntsheet can safely be used as an input.
 */
std::vector<std::string> validateSheetForTransitionSolver(const CalChart::Sheet& sheet);

/*!
 * @brief Solve the transition between the two provided stuntsheets.
 * @param sheet1 The stuntsheet providing the start locations for the transition.
 * @param sheet2 The stuntsheet providing the destination locations for the transition.
 * @param params The parameters used to give constraints for how the transition should be solved.
 * @param delegate An object that will be notified about the progress of the transition
 * solver as it runs, and that can take some role in deciding when the task should abort.
 * @result The solution for the transition between the provided stuntsheets.
 */
TransitionSolverResult runTransitionSolver(const CalChart::Sheet& sheet1, const CalChart::Sheet& sheet2, TransitionSolverParams params, TransitionSolverDelegate* delegate);
}
