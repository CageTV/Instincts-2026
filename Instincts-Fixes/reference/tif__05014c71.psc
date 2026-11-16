ScriptName TIF__05014C71 Extends TopicInfo hidden

;-- Variables ---------------------------------------

;-- Properties --------------------------------------
Furniture Property TrainFurniture Auto

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Function Fragment_0(ObjectReference akSpeakerRef)
  Actor akSpeaker = akSpeakerRef as Actor ; #DEBUG_LINE_NO:7
  Quest owningQuest = Self.GetOwningQuest() ; #DEBUG_LINE_NO:9
  Int sitStatus = (owningQuest as mumirnik_quest_usefurniture).UseFurniture(TrainFurniture) ; #DEBUG_LINE_NO:10
  If sitStatus >= 0 ; #DEBUG_LINE_NO:11
    (owningQuest as mumirnik_quest_instincts_pettraining).Train(akSpeaker) ; #DEBUG_LINE_NO:12
  EndIf
EndFunction
