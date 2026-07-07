#pragma once

#include <RLGym/RocketLeague/Framework.h>
#include <RLGym/RocketLeague/RocketSim/GameState.h>
#include <RLGym/RocketLeague/RocketSim/typing.h>

#include <RLGym/API/typing.h>

#include <vector>
#include <array>
#include <string>

using namespace RLGYM_API_NS;
using namespace RLGYM_RL_NS::RGSim;

START_RL_NS(ActionParsers)

	template <typename AgentID>
class LookupTableAction : public ActionParser<AgentID, int, RocketSimAction, GameState<AgentID>, std::tuple<std::string, int>> {
public:
	LookupTableAction() { this->MakeLookupTable(); };
	std::tuple<std::string, int> GetActionSpace(const AgentID& agent) override { return std::make_tuple(std::string("discrete"), static_cast<int>(this->m_lookupTable.size())); };
	AGENT_MAP(RocketSimAction) ParseActions(const AGENT_MAP(int) actions, GameState<AgentID>& state, SharedInfo& sharedInfo) override {
		AGENT_MAP(RocketSimAction) parsedActions = {};

		for (const auto& [agent, action] : actions) {
			RocketSimAction parsedAction = {};
			parsedAction.push_back(this->m_lookupTable[action]);
			parsedActions.emplace(agent, parsedAction);
		}

		return parsedActions;
		;
	}
	void Reset(const std::vector<AgentID> agents, GameState<AgentID>& initialState, SharedInfo& sharedInfo) override {}; // Noop

private:
	std::vector<FloatArray8> m_lookupTable = {};

	void MakeLookupTable() {
		std::vector<FloatArray8> actions = {};

		std::array<float, 3> F_B = { -1, 0, 1 }; // Float bins
		std::array<float, 2> B_B = { 0, 1 }; // Boolean bins

		// Ground actions
		for (float throttle : F_B) {
			for (float steer : F_B) {
				for (float boost : B_B) {
					for (float handbrake : B_B) {
						if (boost == 1 and throttle != 1) continue;// Remove no throttle actions if boosting (because boosting auto activate throttle)
						actions.push_back(
							{ float(throttle or boost), steer, 0, steer, 0, 0, boost, handbrake }
						);
					}
				}
			}
		}

		// Aerial actions
		for (float pitch : F_B) {
			for (float yaw : F_B) {
				for (float roll : F_B) {
					for (float jump : B_B) {
						for (float boost : B_B) {
							if (jump == 1 and yaw != 0) continue; // Only need roll for sideflip
							if (pitch == 0 and roll == 0 and jump == 0) continue; // Duplicate with ground

							float handbrake = jump == 1 and (pitch != 0 or yaw != 0 or roll != 0);
							actions.push_back({ boost, yaw, pitch, yaw, roll, jump, boost, handbrake });
						}
					}
				}
			}
		}

		this->m_lookupTable = actions;
	}
};

END_RL_NS