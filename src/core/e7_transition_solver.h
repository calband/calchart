//
//  e7_transition_solver.hpp
//  CalChart
//
//  Created by Kevin Durand on 10/12/16.
//
//

#pragma once

#include <array>
#include <vector>
#include "cc_coord.h"
#include "cc_sheet.h"

struct TransitionSolverParams {
    
    enum AlgorithmIdentifier {
        BEGIN = 0,
        E7_ALGORITHM__CHIU_ZAMORA_MALANI = BEGIN,
        E7_ALGORITHM__NAMINIASL_RAMIREZ_ZHANG,
        E7_ALGORITHM__SOVER_ELICEIRI_HERSHKOVITZ,
        END,
    };
    
    struct GroupParams {
        GroupParams() {};
        
        std::set<unsigned> marchers;
        std::set<unsigned> allowedDestinations;
    };
    
    struct InstructionOption {
        enum Pattern {
            BEGIN = 0,
            EWNS = BEGIN,
            NSEW,
            DMHS,
            HSDM,
            END
        };
        
        InstructionOption() : movementPattern(EWNS), waitBeats(0) {};
        
        Pattern movementPattern;
        unsigned waitBeats;
    };
    
    TransitionSolverParams() { for (size_t i = 0; i < availableInstructionsMask.size(); i++) { availableInstructionsMask[i] = false; } };
    
    AlgorithmIdentifier algorithm;
    std::vector<GroupParams> groups;
    std::array<InstructionOption, 8> availableInstructions;
    std::array<bool, 8> availableInstructionsMask;
};

struct TransitionSolverResult {
    
    bool successfullySolved;
    unsigned numBeatsOfMovement;
    
    std::vector<CC_coord> finalPositions;
    std::map<SYMBOL_TYPE, std::string> continuities;
    std::vector<SYMBOL_TYPE> marcherDotTypes;
};

class TransitionSolverDelegate {
public:
    virtual void OnProgress(double progress) = 0;
    virtual void OnSubtaskProgress(double progress) = 0;
    virtual void OnNewPreferredSolution(unsigned numBeatsInSolution) = 0;
    virtual void OnCalculationComplete(TransitionSolverResult finalSolution) = 0;
    virtual bool ShouldAbortCalculation() = 0;
};

std::vector<std::string> validateSheetForTransitionSolver(const CC_sheet& sheet);
TransitionSolverResult runTransitionSolver(const CC_sheet& sheet1, const CC_sheet& sheet2, TransitionSolverParams params, TransitionSolverDelegate *delegate);
