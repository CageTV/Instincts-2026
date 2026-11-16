ScriptName TIF__0500AA1A Extends TopicInfo hidden

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Function Fragment_0(ObjectReference akSpeakerRef)
  Actor akSpeaker = akSpeakerRef as Actor ; #DEBUG_LINE_NO:7
  (Self.GetOwningQuest() as mumirnik_quest_instincts_behavior).WaitForPlayer(akSpeaker, True) ; #DEBUG_LINE_NO:9
EndFunction
