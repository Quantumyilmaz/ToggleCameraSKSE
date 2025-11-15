#pragma once
#include "ClibUtil/singleton.hpp"

enum class PlayerFlags : std::uint8_t {kNone,kInCombat};

class Manager : public clib_util::singleton::ISingleton<Manager> {
	SKSE::stl::enumeration<PlayerFlags, std::uint32_t> flags;
	bool SetFlag(PlayerFlags a_flag, bool a_set);
	void Toggle();
	void Zoom();

	void OnCombatStateChanged(bool a_entered);
public:
	void CheckCombat();
};