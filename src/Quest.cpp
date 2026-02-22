#include "Quest.h"

void Quest::CompleteQuest(RE::FormID questId) {
    using func_t = int32_t(RE::TESQuest*, bool);
    auto quest = RE::TESQuest::LookupByID<RE::TESQuest>(questId);
    const REL::Relocation<func_t> func{REL::RelocationID(24472, 24991)};
    func(quest, true);
}

class OneUIntArgument : public RE::BSScript::IFunctionArguments {
public:
    OneUIntArgument(uint32_t a_value) : value(a_value) {}

    bool operator()(RE::BSScrapArray<RE::BSScript::Variable>& a_dst) const override {
        a_dst.resize(1);
        a_dst[0].SetSInt(value);
        return true;
    }

private:
    int value;
};
bool Quest::IsDone(RE::FormID formId) {
    auto questForm = RE::TESForm::LookupByID<RE::TESQuest>(formId);
    return questForm->IsCompleted();
}
void Quest::SetQuestSage(RE::FormID formId, std::vector<uint32_t> stages) {
    auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();

    auto questForm = RE::TESForm::LookupByID<RE::TESQuest>(formId);

    if (!questForm) return;

    auto policy = vm->GetObjectHandlePolicy();
    RE::VMHandle handle = policy->GetHandleForObject(questForm->GetFormType(), questForm);
    if (handle == policy->EmptyHandle()) {
        return;
    }
    for (auto stage : stages) {
        OneUIntArgument args(stage);
        RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
        auto res = vm->DispatchMethodCall(handle, "Quest", "SetCurrentStageID", &args, callback);
    }
}

class OneObjectRefArgument : public RE::BSScript::IFunctionArguments {
public:
    OneObjectRefArgument(RE::TESObjectREFR* a_ref) : ref(a_ref) {}
    bool operator()(RE::BSScrapArray<RE::BSScript::Variable>& a_dst) const override {
        a_dst.resize(1);
        // We need to set the variable as an object (reference)
        auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
        auto policy = vm->GetObjectHandlePolicy();
        RE::VMHandle refHandle = policy->GetHandleForObject(ref->GetFormType(), ref);
        RE::BSTSmartPointer<RE::BSScript::Object> refObject;
        vm->FindBoundObject(refHandle, "ObjectReference", refObject);
        a_dst[0].SetObject(refObject);
        return true;
    }

private:
    RE::TESObjectREFR* ref;
};

void Quest::ForceAliasRef(RE::FormID questFormId, uint32_t aliasId, RE::FormID targetRefId) {
    auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();

    auto questForm = RE::TESForm::LookupByID<RE::TESQuest>(questFormId);
    auto targetRef = RE::TESForm::LookupByID<RE::TESObjectREFR>(targetRefId);
    if (!questForm || !targetRef) return;

    // Get the alias from the quest
    RE::BGSBaseAlias* alias = nullptr;
    for (auto& a : questForm->aliases) {
        if (a && a->aliasID == aliasId) {
            alias = a;
            break;
        }
    }
    if (!alias) {
        return;
    }

    auto policy = vm->GetObjectHandlePolicy();
    RE::VMHandle handle = policy->GetHandleForObject(alias->GetVMTypeID(), alias);
    if (handle == policy->EmptyHandle()) {
        return;
    }

    OneObjectRefArgument args(targetRef);
    RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
    auto res = vm->DispatchMethodCall(handle, "ReferenceAlias", "ForceRefTo", &args, callback);
}