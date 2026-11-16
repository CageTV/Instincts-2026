ScriptName TIF__05060D21 Extends TopicInfo hidden

;-- Variables ---------------------------------------

;-- Properties --------------------------------------
Message Property MountedWeaponsMessageFAILED Auto

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Function Fragment_0(ObjectReference akSpeakerRef)
  Actor akSpeaker = akSpeakerRef as Actor ; #DEBUG_LINE_NO:7
  If Game.GetPlayer().IsWeaponDrawn() ; #DEBUG_LINE_NO:9
    MountedWeaponsMessageFAILED.Show(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0) ; #DEBUG_LINE_NO:10
  Else
    akSpeaker.SetDoingFavor(False) ; #DEBUG_LINE_NO:12
    (Self.GetOwningQuest() as mumirnik_quest_instincts_riding).Ride(akSpeaker) ; #DEBUG_LINE_NO:13
  EndIf
EndFunction
