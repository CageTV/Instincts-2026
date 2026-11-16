ScriptName TIF__0504C847 Extends TopicInfo hidden

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Function Fragment_0(ObjectReference akSpeakerRef)
  Actor akSpeaker = akSpeakerRef as Actor ; #DEBUG_LINE_NO:7
  (Self.GetOwningQuest() as mumirnik_quest_instincts_petstats).ShowPetStats(akSpeaker, -1) ; #DEBUG_LINE_NO:9
EndFunction
