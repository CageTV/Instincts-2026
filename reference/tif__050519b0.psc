ScriptName TIF__050519B0 Extends TopicInfo hidden

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Function Fragment_0(ObjectReference akSpeakerRef)
  Actor akSpeaker = akSpeakerRef as Actor ; #DEBUG_LINE_NO:7
  akSpeaker.SetActorValue("HeavyArmorSkillAdvance", 1 as Float) ; #DEBUG_LINE_NO:9
  akSpeaker.StopCombat() ; #DEBUG_LINE_NO:10
  akSpeaker.EvaluatePackage() ; #DEBUG_LINE_NO:11
EndFunction
