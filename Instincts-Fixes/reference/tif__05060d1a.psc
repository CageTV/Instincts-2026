ScriptName TIF__05060D1A Extends TopicInfo hidden

;-- Variables ---------------------------------------

;-- Properties --------------------------------------
Message Property RehomeMessageCONFIRM Auto

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Function Fragment_0(ObjectReference akSpeakerRef)
  Actor akSpeaker = akSpeakerRef as Actor ; #DEBUG_LINE_NO:7
  If RehomeMessageCONFIRM.Show(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0) == 0 ; #DEBUG_LINE_NO:9
    (Self.GetOwningQuest() as mumirnik_quest_instincts_housemarker).MarkLocation(akSpeaker) ; #DEBUG_LINE_NO:10
  EndIf
EndFunction
