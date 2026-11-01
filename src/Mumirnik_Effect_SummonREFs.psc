ScriptName Mumirnik_Effect_SummonREFs Extends activemagiceffect
{Summon Companions: moves every tamed pet to the target. Fix: the original loop ran to ActorREFs.Length - 1 and so never summoned the last pet (the third), which looked like a pet that "won't come".}

;-- Properties --------------------------------------
ReferenceAlias[] Property ActorREFs Auto
VisualEffect Property SummonEffect Auto
Float Property SummonEffectDuration Auto

;-- Functions ---------------------------------------
Event OnEffectStart(Actor akTarget, Actor akCaster)
	Int i = 0
	While i < ActorREFs.Length
		Actor actorToTeleport = ActorREFs[i].GetReference() as Actor
		If actorToTeleport
			actorToTeleport.MoveTo(akTarget as ObjectReference, 0.0, 0.0, 0.0, True)
			actorToTeleport.SetActorValue("WaitingForPlayer", 0 as Float)
			SummonEffect.Play(actorToTeleport as ObjectReference, SummonEffectDuration, None)
		EndIf
		i += 1
	EndWhile
EndEvent
