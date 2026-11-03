ScriptName TIF__05014C78 Extends TopicInfo hidden

;-- "Do something for me." Beast Harmony: instead of going straight into the game's favor mode, the pet's command list opens
;-- (Gather ..., "Something else" = the old favor mode, "Never mind").

GlobalVariable Property CurrentPetIsEatingGlobal Auto

Function Fragment_0(ObjectReference akSpeakerRef)
  Actor akSpeaker = akSpeakerRef as Actor
  CurrentPetIsEatingGlobal.SetValue(0 as Float)
  BeastHarmony_Menu.Open(akSpeaker)
EndFunction
