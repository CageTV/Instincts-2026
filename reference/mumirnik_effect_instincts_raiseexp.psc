ScriptName Mumirnik_Effect_Instincts_RaiseExp Extends activemagiceffect

;-- Variables ---------------------------------------

;-- Properties --------------------------------------
GlobalVariable Property ExperienceAmount Auto
Quest Property PetOptionsQuest Auto

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Event OnEffectStart(Actor akTarget, Actor akCaster)
  (PetOptionsQuest as mumirnik_quest_instincts_pettraining).Progress(akCaster, ExperienceAmount.GetValue()) ; #DEBUG_LINE_NO:8
EndEvent
