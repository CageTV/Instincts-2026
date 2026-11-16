ScriptName Mumirnik_Quest_Instincts_Feeding Extends Quest
{ Handles pet feeding. Exposes functions to start feeding or receive food. }

;-- Variables ---------------------------------------

;-- Properties --------------------------------------
Sound Property EatSound Auto
FormList Property EmptyList Auto
FormList Property FoodSublists Auto
FormList Property FoodTypes Auto
FormList Property FullList Auto
Float Property GoldToHungerIngredientAdd Auto
{ This is added to the gold cost of an ingredient to determine its hunger restore. }
Float Property GoldToHungerIngredientMult Auto
{ This is multiplied by the gold cost of an ingredient to determine its hunger restore. }
Float Property GoldToHungerPotionAdd Auto
{ This is added to the gold cost of a potion to determine its hunger restore. }
Float Property GoldToHungerPotionMult Auto
{ This is multiplied by the gold cost of a potion to determine its hunger restore. }

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Function Feed(Actor akTarget)
{ Open the inventory dialog box to feed target pet. Will throw if the pet is an invalid race or does not have a food list. }
  Int actorId = ((Self as Quest) as mumirnik_quest_instincts_petoptions).GetRaceIdForTamedAnimal(akTarget) ; #DEBUG_LINE_NO:22
  GlobalVariable foodType = FoodTypes.GetAt(actorId) as GlobalVariable ; #DEBUG_LINE_NO:23
  FormList foodListForActor = FoodSublists.GetAt(foodType.GetValue() as Int) as FormList ; #DEBUG_LINE_NO:24
  If !foodListForActor ; #DEBUG_LINE_NO:25
    Debug.MessageBox("Error: No food list for actor") ; #DEBUG_LINE_NO:26
    Return  ; #DEBUG_LINE_NO:27
  EndIf
  akTarget.ModActorValue("CarryWeight", 100 as Float) ; #DEBUG_LINE_NO:30
  akTarget.AddInventoryEventFilter(foodListForActor as Form) ; #DEBUG_LINE_NO:31
  akTarget.ShowGiftMenu(True, foodListForActor, True, False) ; #DEBUG_LINE_NO:32
  akTarget.ModActorValue("CarryWeight", -100 as Float) ; #DEBUG_LINE_NO:33
EndFunction

FormList Function GetFoodListForOriginalRace(Actor akTarget)
{ Gets the food list for the target actor's race. }
  FormList originalRaceList = ((Self as Quest) as mumirnik_quest_instincts_petoptions).originalRaceList ; #DEBUG_LINE_NO:38
  Int raceId = -1 ; #DEBUG_LINE_NO:39
  raceId = originalRaceList.Find(akTarget.GetRace() as Form) ; #DEBUG_LINE_NO:40
  If raceId == -1 ; #DEBUG_LINE_NO:41
    Debug.MessageBox("Error: Unknown actor race") ; #DEBUG_LINE_NO:42
    Return EmptyList ; #DEBUG_LINE_NO:43
  EndIf
  GlobalVariable foodType = FoodTypes.GetAt(raceId) as GlobalVariable ; #DEBUG_LINE_NO:46
  FormList foodListForRace = FoodSublists.GetAt(foodType.GetValue() as Int) as FormList ; #DEBUG_LINE_NO:47
  If !foodListForRace ; #DEBUG_LINE_NO:48
    Return FullList ; #DEBUG_LINE_NO:49
  EndIf
  Return foodListForRace ; #DEBUG_LINE_NO:51
EndFunction

Function ReceiveFood(Actor akTarget, Form afFoodItem, Int aiCount)
{ Called from the pet REF that is receiving food. Handles vfx and stat increases. }
  EatSound.Play(akTarget as ObjectReference) ; #DEBUG_LINE_NO:56
  Float itemValue = 0 as Float ; #DEBUG_LINE_NO:58
  If afFoodItem as Ingredient ; #DEBUG_LINE_NO:59
    itemValue = (afFoodItem.GetGoldValue() as Float * GoldToHungerIngredientMult) + GoldToHungerIngredientAdd ; #DEBUG_LINE_NO:60
  ElseIf afFoodItem as Potion ; #DEBUG_LINE_NO:61
    itemValue = (afFoodItem.GetGoldValue() as Float * GoldToHungerPotionMult) + GoldToHungerPotionAdd ; #DEBUG_LINE_NO:62
  Else
    itemValue = afFoodItem.GetGoldValue() as Float ; #DEBUG_LINE_NO:64
  EndIf
  itemValue *= aiCount as Float ; #DEBUG_LINE_NO:66
  Int slotNumber = ((Self as Quest) as mumirnik_quest_instincts_petoptions).GetSlotNumberForActor(akTarget) ; #DEBUG_LINE_NO:68
  ((Self as Quest) as mumirnik_quest_instincts_petstats).ModHunger(akTarget, (itemValue as Int) as Float) ; #DEBUG_LINE_NO:69
  ((Self as Quest) as mumirnik_quest_instincts_petstats).HungerChangeMessage[slotNumber].Show(-itemValue, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0) ; #DEBUG_LINE_NO:70
  akTarget.RemoveItem(afFoodItem, aiCount, False, None) ; #DEBUG_LINE_NO:72
EndFunction
