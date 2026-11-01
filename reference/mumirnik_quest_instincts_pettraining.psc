ScriptName Mumirnik_Quest_Instincts_PetTraining Extends Quest
{ Handles pet training and progression. Exposes functions to initialize progression, train the pet, and undo all progression. }

;-- Variables ---------------------------------------

;-- Properties --------------------------------------
Keyword Property IsUnaggressivePetRace Auto
Message Property LevelUpMessage Auto
String Property PetLevelAVName Auto
{ The name of the actor value that tracks the "level" the target pet is currently at at. This does not correspond to their real level for technical reasons. It caps out at PetLevelProgressAVName. }
String Property PetLevelAvailableFlagAVName Auto
{ The name of the actor value that flags the target pet as being able to level up. }
String Property PetLevelProgressAVName Auto
{ The name of the actor value that tracks the "level" progression of the target pet. This creeps up as you do things that increase its "level". The maximum amount of "levels" the pet can get is equal to Floor(this). }
FormList Property PetLevelsList Auto
{ The level for each pet race/original race. }
Message[] Property PetStatisticsTrainingMessage Auto
Message[] Property PetStatisticsTrainingMessageUnaggressive Auto
Actor Property PlayerREF Auto
Message[] Property ProgressionAvailableMessage Auto
GlobalVariable Property SpeechcraftExperienceGainTraining Auto
Message Property TrainingConfirmMessageAttackDamage Auto
Message Property TrainingConfirmMessageCarryWeight Auto
Message Property TrainingConfirmMessageHealth Auto
Message Property TrainingConfirmMessageHungerFortify Auto
GlobalVariable Property TrainingIncrementAttackDamageGlobal Auto
GlobalVariable Property TrainingIncrementCarryWeightGlobal Auto
GlobalVariable Property TrainingIncrementHealthGlobal Auto
GlobalVariable Property TrainingIncrementHungerFortifyGlobal Auto

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Function InitTrainingProgression(Actor akTarget, Int aiRaceId)
{ Initializes training progression when the animal is tamed. }
  Int level = (PetLevelsList.GetAt(aiRaceId) as GlobalVariable).GetValue() as Int ; #DEBUG_LINE_NO:33
  akTarget.SetActorValue(PetLevelAVName, level as Float) ; #DEBUG_LINE_NO:34
  akTarget.SetActorValue(PetLevelProgressAVName, level as Float) ; #DEBUG_LINE_NO:35
  akTarget.SetActorValue(PetLevelAvailableFlagAVName, 0.0) ; #DEBUG_LINE_NO:36
EndFunction

Function Train(Actor akTarget)
{ Shows the training dialog for a pet and handles the player's selections and resulting progression. }
  Int slotNumber = ((Self as Quest) as mumirnik_quest_instincts_petoptions).GetSlotNumberForActor(akTarget) ; #DEBUG_LINE_NO:41
  If slotNumber != -1 ; #DEBUG_LINE_NO:42
    String hungerFortifyAVName = ((Self as Quest) as mumirnik_quest_instincts_petstats).hungerFortifyAVName ; #DEBUG_LINE_NO:43
    Int petLevelTotal = akTarget.GetActorValue(PetLevelAVName) as Int ; #DEBUG_LINE_NO:44
    Float petDamageTotal = akTarget.GetActorValue("UnarmedDamage") ; #DEBUG_LINE_NO:45
    Float petHealthTotal = akTarget.GetActorValue("Health") ; #DEBUG_LINE_NO:46
    Float petHungerFortifyTotal = akTarget.GetActorValue(hungerFortifyAVName) ; #DEBUG_LINE_NO:47
    Float petCarryWeightTotal = akTarget.GetActorValue("CarryWeight") ; #DEBUG_LINE_NO:48
    Int petLevelAdd = 0 ; #DEBUG_LINE_NO:49
    Float petDamageAdd = 0 as Float ; #DEBUG_LINE_NO:50
    Float petHealthAdd = 0 as Float ; #DEBUG_LINE_NO:51
    Float petHungerFortifyAdd = 0 as Float ; #DEBUG_LINE_NO:52
    Float petCarryWeightAdd = 0 as Float ; #DEBUG_LINE_NO:53
    Float petLevelProgress = akTarget.GetActorValue(PetLevelProgressAVName) ; #DEBUG_LINE_NO:55
    Float trainingIncrementAttackDamage = TrainingIncrementAttackDamageGlobal.GetValue() ; #DEBUG_LINE_NO:57
    Float trainingIncrementHealth = TrainingIncrementHealthGlobal.GetValue() ; #DEBUG_LINE_NO:58
    Float trainingIncrementHungerFortify = TrainingIncrementHungerFortifyGlobal.GetValue() ; #DEBUG_LINE_NO:59
    Float trainingIncrementCarryWeight = TrainingIncrementCarryWeightGlobal.GetValue() ; #DEBUG_LINE_NO:60
    Bool isUnaggressive = akTarget.HasKeyword(IsUnaggressivePetRace) ; #DEBUG_LINE_NO:62
    Int trainingOption = -1 ; #DEBUG_LINE_NO:64
    While trainingOption < 3 && petLevelTotal <= (petLevelProgress as Int - 1) ; #DEBUG_LINE_NO:65
      If !isUnaggressive ; #DEBUG_LINE_NO:66
        trainingOption = PetStatisticsTrainingMessage[slotNumber].Show(petLevelTotal as Float, petDamageTotal, petHealthTotal, petHungerFortifyTotal, 0.0, 0.0, 0.0, 0.0, 0.0) ; #DEBUG_LINE_NO:67
      Else
        trainingOption = PetStatisticsTrainingMessageUnaggressive[slotNumber].Show(petLevelTotal as Float, petCarryWeightTotal, petHealthTotal, petHungerFortifyTotal, 0.0, 0.0, 0.0, 0.0, 0.0) ; #DEBUG_LINE_NO:69
      EndIf
      If trainingOption == 0 ; #DEBUG_LINE_NO:71
        If !isUnaggressive ; #DEBUG_LINE_NO:72
          If TrainingConfirmMessageAttackDamage.Show(petLevelTotal as Float, (petLevelTotal + 1) as Float, petDamageTotal, petDamageTotal + trainingIncrementAttackDamage, 0.0, 0.0, 0.0, 0.0, 0.0) == 0 ; #DEBUG_LINE_NO:73
            petDamageTotal += trainingIncrementAttackDamage ; #DEBUG_LINE_NO:74
            petDamageAdd += trainingIncrementAttackDamage ; #DEBUG_LINE_NO:75
            petLevelTotal += 1 ; #DEBUG_LINE_NO:76
            petLevelAdd += 1 ; #DEBUG_LINE_NO:77
            LevelUpMessage.Show(petLevelTotal as Float, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0) ; #DEBUG_LINE_NO:78
          EndIf
        ElseIf TrainingConfirmMessageCarryWeight.Show(petLevelTotal as Float, (petLevelTotal + 1) as Float, petCarryWeightTotal, petCarryWeightTotal + trainingIncrementCarryWeight, 0.0, 0.0, 0.0, 0.0, 0.0) == 0 ; #DEBUG_LINE_NO:81
          petCarryWeightTotal += trainingIncrementCarryWeight ; #DEBUG_LINE_NO:82
          petCarryWeightAdd += trainingIncrementCarryWeight ; #DEBUG_LINE_NO:83
          petLevelTotal += 1 ; #DEBUG_LINE_NO:84
          petLevelAdd += 1 ; #DEBUG_LINE_NO:85
          LevelUpMessage.Show(petLevelTotal as Float, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0) ; #DEBUG_LINE_NO:86
        EndIf
      ElseIf trainingOption == 1 ; #DEBUG_LINE_NO:89
        If TrainingConfirmMessageHealth.Show(petLevelTotal as Float, (petLevelTotal + 1) as Float, petHealthTotal, petHealthTotal + trainingIncrementHealth, 0.0, 0.0, 0.0, 0.0, 0.0) == 0 ; #DEBUG_LINE_NO:90
          petHealthTotal += trainingIncrementHealth ; #DEBUG_LINE_NO:91
          petHealthAdd += trainingIncrementHealth ; #DEBUG_LINE_NO:92
          petLevelTotal += 1 ; #DEBUG_LINE_NO:93
          petLevelAdd += 1 ; #DEBUG_LINE_NO:94
          LevelUpMessage.Show(petLevelTotal as Float, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0) ; #DEBUG_LINE_NO:95
        EndIf
      ElseIf trainingOption == 2 ; #DEBUG_LINE_NO:97
        If TrainingConfirmMessageHungerFortify.Show(petLevelTotal as Float, (petLevelTotal + 1) as Float, petHungerFortifyTotal, petHungerFortifyTotal + trainingIncrementHungerFortify, 0.0, 0.0, 0.0, 0.0, 0.0) == 0 ; #DEBUG_LINE_NO:98
          petHungerFortifyTotal += trainingIncrementHungerFortify ; #DEBUG_LINE_NO:99
          petHungerFortifyAdd += trainingIncrementHungerFortify ; #DEBUG_LINE_NO:100
          petLevelTotal += 1 ; #DEBUG_LINE_NO:101
          petLevelAdd += 1 ; #DEBUG_LINE_NO:102
          LevelUpMessage.Show(petLevelTotal as Float, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0) ; #DEBUG_LINE_NO:103
        EndIf
      EndIf
    EndWhile
    If petLevelAdd > 0 ; #DEBUG_LINE_NO:108
      akTarget.ModActorValue(PetLevelAVName, petLevelAdd as Float) ; #DEBUG_LINE_NO:109
      Game.AdvanceSkill("Speechcraft", SpeechcraftExperienceGainTraining.GetValue() * petLevelAdd as Float) ; #DEBUG_LINE_NO:110
    EndIf
    If petDamageAdd > 0 as Float ; #DEBUG_LINE_NO:112
      akTarget.ModActorValue("UnarmedDamage", petDamageAdd) ; #DEBUG_LINE_NO:113
    EndIf
    If petHealthAdd > 0 as Float ; #DEBUG_LINE_NO:115
      akTarget.ModActorValue("Health", petHealthAdd) ; #DEBUG_LINE_NO:116
    EndIf
    If petHungerFortifyAdd > 0 as Float ; #DEBUG_LINE_NO:118
      akTarget.ModActorValue(hungerFortifyAVName, petHungerFortifyAdd) ; #DEBUG_LINE_NO:119
    EndIf
    If petCarryWeightAdd > 0 as Float ; #DEBUG_LINE_NO:121
      akTarget.ModActorValue("CarryWeight", petCarryWeightAdd) ; #DEBUG_LINE_NO:122
    EndIf
    If petLevelTotal < petLevelProgress as Int ; #DEBUG_LINE_NO:125
      akTarget.SetActorValue(PetLevelAvailableFlagAVName, 1.0) ; #DEBUG_LINE_NO:126
    Else
      akTarget.SetActorValue(PetLevelAvailableFlagAVName, 0.0) ; #DEBUG_LINE_NO:128
    EndIf
  EndIf
EndFunction

Function Progress(Actor akTarget, Float afValue)
{ Gain training progression by advancing PetLevelProgress. This increases the maximum value up to where you can train the pet. Also displays a message. Throws if progress value is negative. }
  If afValue <= 0 as Float ; #DEBUG_LINE_NO:138
    Debug.MessageBox("Error: Progress must be positive, received " + afValue as String) ; #DEBUG_LINE_NO:139
    Return  ; #DEBUG_LINE_NO:140
  EndIf
  Float petLevel = akTarget.GetActorValue(PetLevelAVName) ; #DEBUG_LINE_NO:143
  Float playerLevel = PlayerREF.GetLevel() as Float ; #DEBUG_LINE_NO:144
  If petLevel >= playerLevel ; #DEBUG_LINE_NO:145
    Return  ; #DEBUG_LINE_NO:146
  EndIf
  String experienceFortifyAVName = ((Self as Quest) as mumirnik_quest_instincts_petstats).experienceFortifyAVName ; #DEBUG_LINE_NO:149
  Float value = afValue * (100 as Float + akTarget.GetActorValue(experienceFortifyAVName)) / 100 as Float ; #DEBUG_LINE_NO:150
  Float oldPetLevelProgress = akTarget.GetActorValue(PetLevelProgressAVName) ; #DEBUG_LINE_NO:152
  If value > playerLevel - oldPetLevelProgress ; #DEBUG_LINE_NO:153
    value = playerLevel - oldPetLevelProgress ; #DEBUG_LINE_NO:154
  EndIf
  Float newPetLevelProgress = oldPetLevelProgress + value ; #DEBUG_LINE_NO:157
  If oldPetLevelProgress as Int < newPetLevelProgress as Int ; #DEBUG_LINE_NO:159
    Int slotNumber = ((Self as Quest) as mumirnik_quest_instincts_petoptions).GetSlotNumberForActor(akTarget) ; #DEBUG_LINE_NO:160
    If slotNumber != -1 ; #DEBUG_LINE_NO:161
      ProgressionAvailableMessage[slotNumber].Show(newPetLevelProgress, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0) ; #DEBUG_LINE_NO:162
    EndIf
  EndIf
  akTarget.ModActorValue(PetLevelProgressAVName, value) ; #DEBUG_LINE_NO:166
  If petLevel as Int < newPetLevelProgress as Int ; #DEBUG_LINE_NO:168
    akTarget.SetActorValue(PetLevelAvailableFlagAVName, 1.0) ; #DEBUG_LINE_NO:169
  Else
    akTarget.SetActorValue(PetLevelAvailableFlagAVName, 0.0) ; #DEBUG_LINE_NO:171
  EndIf
EndFunction
