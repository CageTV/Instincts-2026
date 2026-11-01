ScriptName Mumirnik_Quest_AddSpellsToListsSetup Extends Quest

;-- Variables ---------------------------------------

;-- Properties --------------------------------------
Actor Property PlayerRef Auto
Float Property RunDelay Auto
{ This is used in some other people's scripts, I assume it prevents the cart glitch. }
Scroll[] Property SpellScrollsList Auto
LeveledItem[] Property SpellScrollsLootItemsList Auto
Weapon[] Property SpellStavesList Auto
LeveledItem[] Property SpellStavesLootItemsList Auto
Book[] Property SpellTomesList Auto
LeveledItem[] Property SpellTomesLootItemsList Auto
FormList[] Property SpellTomesSpecialFormListsList Auto
Book[] Property SpellTomesSpecialList Auto
{ Master spell tomes are added in a form list. }

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Event OnInit()
  Self.RegisterForSingleUpdate(RunDelay) ; #DEBUG_LINE_NO:19
EndEvent

Event OnUpdate()
  Debug.Trace("Start spell injection", 0) ; #DEBUG_LINE_NO:23
  If SpellTomesList.Length != SpellTomesLootItemsList.Length || SpellStavesList.Length != SpellStavesLootItemsList.Length || SpellScrollsList.Length != SpellScrollsLootItemsList.Length ; #DEBUG_LINE_NO:24
    Debug.MessageBox("Error: Spell lists don't have the same length, spells will not be added to leveled items") ; #DEBUG_LINE_NO:25
    Return  ; #DEBUG_LINE_NO:26
  EndIf
  Int i = 0 ; #DEBUG_LINE_NO:28
  While i < SpellTomesLootItemsList.Length - 1 ; #DEBUG_LINE_NO:29
    SpellTomesLootItemsList[i].AddForm(SpellTomesList[i] as Form, 1, 1) ; #DEBUG_LINE_NO:30
    i += 1 ; #DEBUG_LINE_NO:31
  EndWhile
  Int j = 0 ; #DEBUG_LINE_NO:33
  While j < SpellStavesLootItemsList.Length - 1 ; #DEBUG_LINE_NO:34
    SpellStavesLootItemsList[j].AddForm(SpellStavesList[j] as Form, 1, 1) ; #DEBUG_LINE_NO:35
    j += 1 ; #DEBUG_LINE_NO:36
  EndWhile
  Int k = 0 ; #DEBUG_LINE_NO:38
  While k < SpellScrollsLootItemsList.Length - 1 ; #DEBUG_LINE_NO:39
    SpellScrollsLootItemsList[k].AddForm(SpellScrollsList[k] as Form, 1, 1) ; #DEBUG_LINE_NO:40
    k += 1 ; #DEBUG_LINE_NO:41
  EndWhile
  Int l = 0 ; #DEBUG_LINE_NO:43
  While l < SpellTomesSpecialList.Length - 1 ; #DEBUG_LINE_NO:44
    SpellTomesSpecialFormListsList[l].AddForm(SpellTomesSpecialList[l] as Form) ; #DEBUG_LINE_NO:45
    l += 1 ; #DEBUG_LINE_NO:46
  EndWhile
  Debug.Trace("Finish spell injection", 0) ; #DEBUG_LINE_NO:48
EndEvent
