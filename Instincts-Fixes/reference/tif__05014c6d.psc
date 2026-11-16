ScriptName TIF__05014C6D Extends TopicInfo hidden

;-- Variables ---------------------------------------

;-- Properties --------------------------------------
Spell Property HeartbreakSpell Auto
Actor Property PlayerRef Auto
Message Property ReleaseMessageCONFIRM Auto

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Function Fragment_0(ObjectReference akSpeakerRef)
  Actor akSpeaker = akSpeakerRef as Actor ; #DEBUG_LINE_NO:7
  If ReleaseMessageCONFIRM.Show(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0) == 0 ; #DEBUG_LINE_NO:9
    (Self.GetOwningQuest() as mumirnik_quest_instincts_petoptions).ReleasePet(akSpeaker) ; #DEBUG_LINE_NO:11
  EndIf
EndFunction
