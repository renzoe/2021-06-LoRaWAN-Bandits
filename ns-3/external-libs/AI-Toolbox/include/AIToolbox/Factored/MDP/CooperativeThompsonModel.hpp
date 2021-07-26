#ifndef AI_TOOLBOX_FACTORED_MDP_THOMPSON_MODEL_HEADER_FILE
#define AI_TOOLBOX_FACTORED_MDP_THOMPSON_MODEL_HEADER_FILE

#include <AIToolbox/Factored/MDP/CooperativeExperience.hpp>
#include <AIToolbox/Factored/Utils/BayesianNetwork.hpp>

namespace AIToolbox::Factored::MDP {
    /**
     * @brief This class models CooperativeExperience as a CooperativeModel using Thompson Sampling.
     *
     * Often an MDP is not known in advance. It is known that it can assume
     * a certain set of states, and that a certain set of actions are
     * available to the agent, but not much more. Thus, in these cases, the
     * goal is not only to find out the best policy for the MDP we have,
     * but at the same time learn the actual transition and reward
     * functions of such a model. This task is called "reinforcement
     * learning".
     *
     * This class helps with this. A naive approach in reinforcement learning
     * is to keep track, for each action, of its results, and deduce transition
     * probabilities and rewards based on the data collected in such a way.
     * This class does just this, using Thompson Sampling to decide what the
     * transition probabilities and rewards are.
     *
     * This class maps a CooperativeExperience object using a series of
     * Dirichlet (for transitions) and Student-t (for rewards) distributions,
     * one per state-action pairs. The user can sample from these distributions
     * to obtain transition and reward functions. As more data is accumulated,
     * the distributions can be resampled so that these functions better
     * reflect the data. The syncing operation MUST be done manually as it is
     * slightly expensive (it must sample a distribution with S parameters and
     * normalize the result). See sync().
     *
     * When little data is available, syncing will generally result in
     * transition functions where most transitions are assumed possible. Priors
     * can be given to the Experience as "fictional" experience so as to bias
     * the result. Additionally, this class uses Jeffreys prior when sampling.
     * For a Dirichlet distribution, this is equivalent to having 0.5 priors on
     * all parameters (which can't be set via the CooperativeExperience, as
     * they are not integers). For the rewards, the posteriors are student-t
     * distribution. A Jeffreys prior ensures that the sampling is non-biased
     * through any transformation of the original parameters.
     *
     * The strength of ThompsonModel is that it can replace traditional
     * exploration techniques, embedding our beliefs of what transitions and
     * rewards are possible directly in the sampled functions.
     *
     * Whether any of these techniques work or not can definitely depend on
     * the model you are trying to approximate. Trying out things is good!
     */
    class CooperativeThompsonModel {
        public:
            using TransitionMatrix   = DDN;
            using RewardMatrix       = std::vector<Vector>;

            /**
             * @brief Constructor using previous Experience.
             *
             * This constructor stores a reference to the CooperativeExperience
             * that will be used to learn an MDP Model from the data, and
             * initializes internal Model data.
             *
             * The user can choose whether he wants to directly sync the
             * CooperativeMaximumLikelihoodModel to the underlying
             * CooperativeExperience, or delay it for later.
             *
             * In the latter case the default transition function defines a
             * transition of probability 1 for each state factor to 0, no
             * matter the action or its parents.
             *
             * In general it would be better to add some amount of bias to the
             * CooperativeExperience so that when a new state-action pair is
             * tried, the CooperativeMaximumLikelihoodModel doesn't
             * automatically compute 100% probability of transitioning to the
             * resulting state, but smooths into it. This may depend on your
             * problem though.
             *
             * The default reward function is 0.
             *
             * @param exp The CooperativeExperience of the model.
             * @param discount The discount used in solving methods.
             * @param sync Whether to sync with the CooperativeExperience immediately or delay it.
             */
            CooperativeThompsonModel(const CooperativeExperience & exp, double discount = 1.0);

            /**
             * @brief This function syncs the whole CooperativeMaximumLikelihoodModel to the underlying CooperativeExperience.
             *
             * Since use cases in AI are very varied, one may not want to
             * update its CooperativeMaximumLikelihoodModel for each single transition
             * experienced by the agent. To avoid this we leave to the user the
             * task of syncing between the underlying CooperativeExperience and
             * the CooperativeMaximumLikelihoodModel, as he/she sees fit.
             *
             * After this function is run the transition and reward functions
             * will accurately reflect the state of the underlying
             * CooperativeExperience.
             */
            void sync();

            /**
             * @brief This function syncs a state-action pair to the underlying CooperativeExperience.
             *
             * @param s The state that needs to be synced.
             * @param a The action that needs to be synced.
             */
            void sync(const State & s, const Action & a);

            /**
             * @brief This function syncs the given indeces to the underlying CooperativeExperience.
             *
             * This function is equivalent to sync(const State &, const Action
             * &), but it avoids recomputing the indeces of the state-action
             * pair. Instead, it uses the ones already computed by the
             * underlying CooperativeExperience during its record() call.
             *
             * This works because the CooperativeExperience and
             * CooperativeMaximumLikelihoodModel use the same factoring of
             * their data structures, and thus the indeces can be used
             * unchanged in both classes.
             *
             * @param indeces The indeces provided by the CooperativeExperience.
             */
            void sync(const CooperativeExperience::Indeces & indeces);

            /**
             * @brief This function samples the MDP with the specified state action pair.
             *
             * This function samples the model for simulate experience. The transition
             * and reward functions are used to produce, from the state action pair
             * inserted as arguments, a possible new state with respective reward.
             * The new state is picked from all possible states that the MDP allows
             * transitioning to, each with probability equal to the same probability
             * of the transition in the model. After a new state is picked, the reward
             * is the corresponding reward contained in the reward function.
             *
             * @param s The state that needs to be sampled.
             * @param a The action that needs to be sampled.
             *
             * @return A tuple containing a new state and a reward.
             */
            std::tuple<State, double> sampleSR(const State & s, const Action & a) const;

            /**
             * @brief This function samples the MDP with the specified state action pair.
             *
             * This function samples the model for simulate experience. The transition
             * and reward functions are used to produce, from the state action pair
             * inserted as arguments, a possible new state with respective reward.
             * The new state is picked from all possible states that the MDP allows
             * transitioning to, each with probability equal to the same probability
             * of the transition in the model. After a new state is picked, the reward
             * is the vector of corresponding rewards contained in the reward function.
             *
             * @param s The state that needs to be sampled.
             * @param a The action that needs to be sampled.
             *
             * @return A tuple containing a new state and a reward.
             */
            std::tuple<State, Rewards> sampleSRs(const State & s, const Action & a) const;

            /**
             * @brief This function samples the MDP with the specified state action pair.
             *
             * this function is equivalent to samplesr(const state &, const action &).
             *
             * the only difference is that it allows to output the new state
             * into a pre-allocated state, avoiding the need for an allocation
             * at every sample.
             *
             * no checks for nullptr are done.
             *
             * @param s The state that needs to be sampled.
             * @param a The action that needs to be sampled.
             * @param s1 The new state.
             *
             * @return The reward for the sampled transition.
             */
            double sampleSR(const State & s, const Action & a, State * s1) const;

            /**
             * @brief This function samples the MDP with the specified state action pair.
             *
             * This function is equivalent to sampleSRs(const State &, const Action &).
             *
             * The only difference is that it allows to output the new State
             * and Rewards into a pre-allocated State and Rewards, avoiding the
             * need for an allocation at every sample.
             *
             * NO CHECKS for nullptr are done.
             *
             * @param s The state that needs to be sampled.
             * @param a The action that needs to be sampled.
             * @param s1 The new state.
             * @param rews The new rewards.
             */
            void sampleSRs(const State & s, const Action & a, State * s1, Rewards * rews) const;

            /**
             * @brief This function returns the stored transition probability for the specified transition.
             *
             * @param s The initial state of the transition.
             * @param a The action performed in the transition.
             * @param s1 The final state of the transition.
             *
             * @return The probability of the specified transition.
             */
            double getTransitionProbability(const State & s, const Action & a, const State & s1) const;

            /**
             * @brief This function returns the stored expected reward for the specified transition.
             *
             * @param s The initial state of the transition.
             * @param a The action performed in the transition.
             * @param s1 The final state of the transition.
             *
             * @return The expected reward of the specified transition.
             */
            double getExpectedReward(const State & s, const Action & a, const State & s1) const;

            /**
             * @brief This function returns the stored expected rewards for the specified transition.
             *
             * This function returns a vector of the size of the state-space.
             * The sum of the vector is the same as the value returned by the
             * getExpectedReward(const State &, const Action &, const State &)
             * function.
             *
             * @param s The initial state of the transition.
             * @param a The action performed in the transition.
             * @param s1 The final state of the transition.
             *
             * @return The expected reward of the specified transition.
             */
            Rewards getExpectedRewards(const State & s, const Action & a, const State & s1) const;

            /**
             * @brief This function returns the stored expected rewards for the specified transition.
             *
             * This function is equivalent to getExpectedReward(const State &,
             * const Action &, const State &).
             *
             * The only difference is that it allows to output the new Rewards
             * into a pre-allocated Rewards, avoiding the need for an
             * allocation at every sample.
             *
             * NO CHECKS for nullptr are done.
             *
             * @param s The initial state of the transition.
             * @param a The action performed in the transition.
             * @param s1 The final state of the transition.
             *
             * @return The expected reward of the specified transition.
             */
            void getExpectedRewards(const State & s, const Action & a, const State & s1, Rewards * rews) const;

            /**
             * @brief This function returns the number of states of the world.
             *
             * @return The total number of states.
             */
            const State & getS() const;

            /**
             * @brief This function returns the number of available actions to the agent.
             *
             * @return The total number of actions.
             */
            const Action & getA() const;

            /**
             * @brief This function sets a new discount factor for the Model.
             *
             * @param d The new discount factor for the Model.
             */
            void setDiscount(double d);

            /**
             * @brief This function returns the currently set discount factor.
             *
             * @return The currently set discount factor.
             */
            double getDiscount() const;

            /**
             * @brief This function enables inspection of the underlying Experience of the RLModel.
             *
             * @return The underlying Experience of the RLModel.
             */
            const CooperativeExperience & getExperience() const;

            /**
             * @brief This function returns the transition matrix for inspection.
             *
             * @return The transition matrix.
             */
            const TransitionMatrix & getTransitionFunction() const;

            /**
             * @brief This function returns the rewards matrix for inspection.
             *
             * @return The rewards matrix.
             */
            const RewardMatrix & getRewardFunction() const;

            /**
             * @brief This function returns the underlying DDNGraph of the CooperativeExperience.
             *
             * @return The underlying DDNGraph.
             */
            const DDNGraph & getGraph() const;

        private:
            /**
             * @brief This function syncs a single row of T and R.
             *
             * This is used internally to avoid duplicating code.
             *
             * @param i The feature to sync.
             * @param j The row to sync.
             */
            void syncRow(size_t i, size_t j);

            const CooperativeExperience & experience_;
            double discount_;

            TransitionMatrix transitions_;
            RewardMatrix rewards_;

            mutable RandomEngine rand_;
    };
}

#endif

