#include "Persistence.h"
#include "FileReader.h"
#include "FileWritter.h"
#include "Manager.h"
#include "SpeachManager.h"

#define VERSION 1

std::string removeEssSuffix(const std::string& input) {
    if (input.size() >= 4 && input.compare(input.size() - 4, 4, ".ess") == 0) {
        return input.substr(0, input.size() - 4);
    }
    return input;
}

void Persistence::Load(std::string fileName) {
    fileName = removeEssSuffix(fileName) + "_Possesion.bin";
    logger::trace("loading: {}", fileName);

    FileReader reader(fileName);

    if (reader.IsOpen()) {
        auto version = reader.ReadUInt32();

        if (version != VERSION) {
            return;
        }
        auto id = reader.ReadFormId();
        SpeechManager::LoadGame(&reader);
        if (id != 0) {
            Manager::SetBaseCharacter(RE::TESForm::LookupByID<RE::Character>(id));
            Manager::OnNewGame();
            Manager::CopyData();
        }
        logger::trace("File exist");
    } else {
        logger::trace("File do not exists");
    }
}

void Persistence::Save(std::string fileName) {
    fileName = removeEssSuffix(fileName) + "_Possesion.bin";
    logger::trace("saving: {}", fileName);

    FileWritter writer(fileName);
    if (writer.IsOpen()) {
        writer.WriteUInt32(VERSION);
        if (Manager::GetBaseCharacter()) {
            writer.WriteFormId(Manager::GetBaseCharacter()->GetFormID());
        } else {
            writer.WriteFormId(0);
        }
        SpeechManager::SaveGame(&writer);
    } else {
        logger::error("failed to open file");
    }
}