#include "SGVersusPlayerStart.h"

void ASGVersusPlayerStart::AuthOccupy(APlayerController* Player)
{
	if (!HasAuthority()) return;
	
	OccupiedBy = Player;
}
