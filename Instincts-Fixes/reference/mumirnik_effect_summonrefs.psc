ScriptName Mumirnik_Effect_SummonREFs Extends activemagiceffect

;-- Variables ---------------------------------------

;-- Properties --------------------------------------
ReferenceAlias[] Property ActorREFs Auto
VisualEffect Property SummonEffect Auto
Float Property SummonEffectDuration Auto

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Event OnEffectStart(Actor akTarget, Actor akCaster)
  Int i = 0 ; #DEBUG_LINE_NO:8
  While i < ActorREFs.Length - 1 ; #DEBUG_LINE_NO:9
    Actor actorToTeleport = ActorREFs[i].GetReference() as Actor ; #DEBUG_LINE_NO:10
    If actorToTeleport ; #DEBUG_LINE_NO:11
      actorToTeleport.MoveTo(akTarget as ObjectReference, 0.0, 0.0, 0.0, True) ; #DEBUG_LINE_NO:12
      actorToTeleport.SetActorValue("WaitingForPlayer", 0 as Float) ; #DEBUG_LINE_NO:13
      SummonEffect.Play(actorToTeleport as ObjectReference, SummonEffectDuration, None) ; #DEBUG_LINE_NO:14
    EndIf
    i += 1 ; #DEBUG_LINE_NO:16
  EndWhile
EndEvent
