ScriptName TIF__0500AA28 Extends TopicInfo hidden

;-- Variables ---------------------------------------

;-- Properties --------------------------------------
GlobalVariable Property CurrentPetIsEatingGlobal Auto

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Function Fragment_0(ObjectReference akSpeakerRef)
  Actor akSpeaker = akSpeakerRef as Actor ; #DEBUG_LINE_NO:7
  CurrentPetIsEatingGlobal.SetValue(1 as Float) ; #DEBUG_LINE_NO:9
  (Self.GetOwningQuest() as mumirnik_quest_instincts_feeding).Feed(akSpeaker) ; #DEBUG_LINE_NO:10
EndFunction
