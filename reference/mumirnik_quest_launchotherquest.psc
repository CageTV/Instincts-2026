ScriptName Mumirnik_Quest_LaunchOtherQuest Extends Quest
{ This quest will launch another quest after a delay. Use this to avoid cart glitches and other game start shenanigans. }

;-- Variables ---------------------------------------

;-- Properties --------------------------------------
Float Property LaunchDelay Auto
Quest Property LaunchQuest Auto

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Event OnInit()
  Self.RegisterForSingleUpdate(LaunchDelay) ; #DEBUG_LINE_NO:9
EndEvent

Event OnUpdate()
  LaunchQuest.Start() ; #DEBUG_LINE_NO:15
EndEvent
