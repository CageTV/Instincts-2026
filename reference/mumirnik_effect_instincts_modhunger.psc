ScriptName Mumirnik_Effect_Instincts_ModHunger Extends activemagiceffect
{ Modifies hunger when applied to a target. }

;-- Variables ---------------------------------------

;-- Properties --------------------------------------
GlobalVariable Property HungerAmount Auto
Quest Property PetOptionsQuest Auto

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Event OnEffectStart(Actor akTarget, Actor akCaster)
  (PetOptionsQuest as mumirnik_quest_instincts_petstats).ModHunger(akCaster, HungerAmount.GetValue()) ; #DEBUG_LINE_NO:9
EndEvent
