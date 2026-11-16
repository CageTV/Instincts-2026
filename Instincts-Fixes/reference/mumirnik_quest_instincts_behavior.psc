ScriptName Mumirnik_Quest_Instincts_Behavior Extends Quest
{ Handles pet behavior from dialogue calls. }

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Function WaitForPlayer(Actor akTarget, Bool abWait)
{ Toggle target between waiting for the player or not. }
  If abWait ; #DEBUG_LINE_NO:6
    akTarget.SetActorValue("WaitingForPlayer", 1 as Float) ; #DEBUG_LINE_NO:7
  Else
    akTarget.SetActorValue("WaitingForPlayer", 0 as Float) ; #DEBUG_LINE_NO:9
  EndIf
  akTarget.EvaluatePackage() ; #DEBUG_LINE_NO:11
EndFunction
