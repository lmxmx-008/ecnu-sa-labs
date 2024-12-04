#ifndef SOLVERIMPL_H
#define SOLVERIMPL_H

#include "Solver.h"
#include "Time.h"

#include <vector>

namespace miniklee {
    class ExecutionState;
    class Expr;
    struct Query;

  /// SolverImpl - Abstract base clase for solver implementations.
    class SolverImpl {
    public:
    SolverImpl() = default;

    SolverImpl(const SolverImpl&) = delete;
    SolverImpl& operator=(const SolverImpl&) = delete;

    virtual ~SolverImpl();

    enum SolverRunStatus {  SOLVER_RUN_STATUS_SUCCESS_SOLVABLE,
                            SOLVER_RUN_STATUS_SUCCESS_UNSOLVABLE,
                            SOLVER_RUN_STATUS_FAILURE,
                            SOLVER_RUN_STATUS_TIMEOUT,
                            SOLVER_RUN_STATUS_FORK_FAILED,
                            SOLVER_RUN_STATUS_INTERRUPTED,
                            SOLVER_RUN_STATUS_UNEXPECTED_EXIT_CODE,
                            SOLVER_RUN_STATUS_WAITPID_FAILED };

    /// computeValidity - Compute a full validity result for the
    /// query.
    ///
    /// The query expression is guaranteed to be non-constant and have
    /// bool type.
    ///
    /// SolverImpl provides a default implementation which uses
    /// computeTruth. Clients should override this if a more efficient
    /// implementation is available.
    ///
    /// \param [out] result - if
    /// \f[ \forall X constraints(X) \to query(X) \f]
    /// then Solver::True,
    /// else if
    /// \f[ \forall X constraints(X) \to \lnot query(X) \f]
    /// then Solver::False,
    /// else
    /// Solver::Unknown
    ///
    /// \return True on success
    virtual bool computeValidity(const Query& query);
    
    /// computeTruth - Determine whether the given query expression is provably true
    /// given the constraints.
    ///
    /// The query expression is guaranteed to be non-constant and have
    /// bool type.
    ///
    /// This method should evaluate the logical formula:
    ///
    /// \f[ \forall X constraints(X) \to query(X) \f]
    ///
    /// Where \f$X\f$ is some assignment, \f$constraints(X)\f$ are the constraints
    /// in the query and \f$query(X)\f$ is the query expression.
    ///
    /// \param [out] isValid - On success, true iff the logical formula is true.
    /// \return True on success
    virtual bool computeTruth(const Query& query, bool &isValid) = 0;

    /// computeValue - Compute a feasible value for the expression.
    ///
    /// The query expression is guaranteed to be non-constant.
    ///
    /// \return True on success
    virtual bool computeValue(const Query& query, ref<Expr> &result) = 0;
    
    /// \sa Solver::getInitialValues()
    virtual bool computeInitialValues(const Query& query,
                                        const std::vector<const SymbolicExpr*> 
                                        &objects,
                                        std::vector<int32_t> 
                                        &values) = 0;
    
    /// getOperationStatusCode - get the status of the last solver operation
    virtual SolverRunStatus getOperationStatusCode() = 0;

    /// getOperationStatusString - get string representation of the operation
    /// status code
    static const char* getOperationStatusString(SolverRunStatus statusCode);

    virtual std::string getConstraintLog(const Query &query) {
        // dummy
        return {};
    }

    virtual void setCoreSolverTimeout(time::Span timeout) {};
};

}

#endif /* SOLVERIMPL_H */
