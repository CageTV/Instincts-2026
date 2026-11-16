ScriptName TIF__050519B3 Extends TopicInfo hidden

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Function Fragment_0(ObjectReference akSpeakerRef)
  Actor akSpeaker = akSpeakerRef as Actor ; #DEBUG_LINE_NO:7
  akSpeaker.SetActorValue("HeavyArmorSkillAdvance", 0 as Float) ; #DEBUG_LINE_NO:9
  akSpeaker.SetActorValue("WaitingForPlayer", 0 as Float) ; #DEBUG_LINE_NO:10
  akSpeaker.EvaluatePackage() ; #DEBUG_LINE_NO:11
EndFunction
