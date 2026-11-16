ScriptName TIF__05014C7E Extends TopicInfo hidden

;-- Variables ---------------------------------------

;-- Properties --------------------------------------
GlobalVariable Property CurrentPetIsEatingGlobal Auto

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Function Fragment_0(ObjectReference akSpeakerRef)
  Actor akSpeaker = akSpeakerRef as Actor ; #DEBUG_LINE_NO:7
  CurrentPetIsEatingGlobal.SetValue(0 as Float) ; #DEBUG_LINE_NO:9
  akSpeaker.OpenInventory(False) ; #DEBUG_LINE_NO:10
EndFunction
