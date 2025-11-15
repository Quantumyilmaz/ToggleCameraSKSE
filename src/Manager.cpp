#include "Manager.h"

bool Manager::SetFlag(PlayerFlags a_flag, bool a_set)
{
	// get current flag value
	bool current = flags.any(a_flag);
	if (current != a_set) {
		if (a_set) {
			flags.set(a_flag);
		}
		else {
			flags.reset(a_flag);
		}
		return true;
	}
	return false;
}

void Manager::CheckCombat()
{
	if (SetFlag(PlayerFlags::kInCombat, player->IsInCombat()))
	{
		OnCombatStateChanged(flags.any(PlayerFlags::kInCombat));
	}
}
