#include "UI.h"
#include "Manager.h"
#include <d3d11.h>
#include "Translation.h"
#include "FormId.h"
#include "Configuration.h"
#include "SpeachManager.h"
#include "Patch.h"

void UI::Register() {
    if (!SKSEMenuFramework::IsInstalled()) {
        return;
    }
    SKSEMenuFramework::SetSection(MOD_NAME);
    SKSEMenuFramework::AddSectionItem(Translation::Get("Config.Config"), RenderMenu);
    SKSEMenuFramework::AddHudElement(NewGame::RenderBackground);
    NewGame::startAsNPCWindow = SKSEMenuFramework::AddWindow(NewGame::RenderStartAsNPC, true);
    NewGame::chooseCharacterWindow = SKSEMenuFramework::AddWindow(NewGame::RenderChooseCharacterWindow, true);
    NewGame::chooseSideWindow = SKSEMenuFramework::AddWindow(NewGame::RenderChooseSideWindow, true);
    NewGame::reviewWindow = SKSEMenuFramework::AddWindow(NewGame::RenderReviewWindow, true);
}
void BeguinWindow(const char* title) {
    auto viewport = ImGuiMCP::GetMainViewport();
    ImGuiMCP::ImVec2 windowSize = ImGuiMCP::ImVec2{viewport->Size.x * 0.6f, viewport->Size.y * 0.6f};
    ImGuiMCP::ImVec2 windowPos = ImGuiMCP::ImVec2{viewport->Pos.x + (viewport->Size.x - windowSize.x) * 0.5f, viewport->Pos.y + (viewport->Size.y - windowSize.y) * 0.5f};
    ImGuiMCP::SetNextWindowPos(windowPos, ImGuiMCP::ImGuiCond_Appearing, {0, 0});
    ImGuiMCP::SetNextWindowSize(windowSize, ImGuiMCP::ImGuiCond_Appearing);
    ImGuiMCP::Begin(title, nullptr, ImGuiMCP::ImGuiWindowFlags_NoCollapse);
}
void UI::NewGame::Next() {
    if (index < maxIndex) {
        index++;
        Update();
    }
}
void UI::NewGame::Previus() {
    if (index > 0) {
        index--;
        Update();
    }
}
void UI::NewGame::Update() {
    startAsNPCWindow->IsOpen = false;
    chooseCharacterWindow->IsOpen = false;
    chooseSideWindow->IsOpen = false;
    reviewWindow->IsOpen = false;
    switch (index) {
        case 1:
            if (startAsNPCWindow) {
                startAsNPCWindow->IsOpen = true;
            }
            break;
        case 2:
            if (chooseCharacterWindow) {
                chooseCharacterWindow->IsOpen = true;
            }
        break;
        case 3:
            if (chooseSideWindow) {
                chooseSideWindow->IsOpen = true;
            }
            break;
        case 4:
            if (chooseSideWindow) {
                reviewWindow->IsOpen = true;
            }
            break;
        case 5:
            index = 0;
            auto success = reinterpret_cast<const SKSE::detail::SKSEMessagingInterface*>(SKSE::GetMessagingInterface())->Dispatch(0, SKSE::MessagingInterface::kNewGame, (void*)RE::TESForm::LookupByID<RE::TESQuest>(0x3372b), sizeof(void*), nullptr);
            if (!success) {
                logger::error("Failed to patch the SKSE event");
            }
            Manager::StartGame();
        break;
    }
}
void __stdcall UI::RenderMenu() {
    if (ImGuiMCPComponents::ToggleButton(Translation::Get("DevConfig.DevMode"), &Configuration::EnableDevMode)) {
        Configuration::Save();
    }
}

void __stdcall UI::NewGame::RenderStartAsNPC() {
    BeguinWindow(Translation::Get("Window1.PlayerOrNpc"));

    ImGuiMCP::ImVec2 region;
    ImGuiMCP::GetContentRegionAvail(&region);

    float contentWidth = region.x * 0.7f;
    float offsetX = (region.x - contentWidth) * 0.5f;

    float totalHeight = ImGuiMCP::GetTextLineHeightWithSpacing() + ImGuiMCP::GetStyle()->ItemSpacing.y + 50.0f;

    float offsetY = (region.y - totalHeight) * 0.5f;

    if (offsetY > 0.0f) ImGuiMCP::SetCursorPosY(ImGuiMCP::GetCursorPosY() + offsetY);

    ImGuiMCP::SetCursorPosX(offsetX);
    ImGuiMCP::BeginGroup();

    ImGuiMCP::ImVec2 halfButton = {contentWidth * 0.5f - 4.0f, 50.0f};

    ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Button, ImGuiMCP::ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
    ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_ButtonHovered, ImGuiMCP::ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
    ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_ButtonActive, ImGuiMCP::ImVec4(0.9f, 0.2f, 0.2f, 1.0f));

    if (ImGuiMCP::Button(Translation::Get("Window1.StartAsPlayer"), halfButton)) {
        Manager::enabled = false;
        startNewGame();
        index = 0;
        Update();
    }

    ImGuiMCP::PopStyleColor(3);

    ImGuiMCP::SameLine();

    if (ImGuiMCP::Button(Translation::Get("Window1.StartAsNpc"), halfButton)) {
        Manager::enabled = true;
        SpeechManager::NewGame();
        Manager::OnNewGame();
        Next();
    }

    ImGuiMCP::EndGroup();
    ImGuiMCP::End();
}

void __stdcall UI::NewGame::RenderChooseCharacterWindow() {
    BeguinWindow(std::format("{}##MenuEntiryFromMod", Translation::Get("Window2.WhoDoYouWantToBe")).c_str());

    static ImGuiMCP::ImGuiTextFilter filter;
    static int currentPage = 0;
    static int pageSize = Configuration::GroupsPerPage;

    if (ImGuiMCP::Button(Translation::Get("Controls.GoBack"))) {
        Previus();
    }
    
    ImGuiMCP::SameLine();

    bool filterChanged = filter.Draw(Translation::Get("Window2.FilterByName"), 500.0f);



    if (filterChanged) currentPage = 0;

    bool wasDevMode = Configuration::EnableDevMode;

    struct NpcEntry {
        RE::Character* item;
        std::string displayName;
        int globalIndex;
    };

    std::map<std::string, std::vector<NpcEntry>> cellGroups;
    std::unordered_map<std::string, int> totalCount;

    for (auto item : Manager::GetAllCharacters()) {
        if (!item) continue;
        if (!item->GetActorBase()) continue;
        std::string key = std::format("{} 0x{:x}", item->GetDisplayFullName(), item->GetFormID());
        if (!filter.PassFilter(key.c_str())) continue;
        totalCount[key]++;
    }

    std::unordered_map<std::string, int> runningIndex;
    int globalIdx = 0;

    for (auto item : Manager::GetAllCharacters()) {
        if (!item) continue;
        auto base = item->GetActorBase();
        if (!base) continue;
        auto parentCellName = item->GetParentCell() ? item->GetParentCell()->GetName() : "Unknown";
        auto parentCellFormId = item->GetParentCell() ? item->GetParentCell()->GetFormID() : (RE::FormID)0;

        std::string key = std::format("{} 0x{:x} {} 0x{:x}", item->GetDisplayFullName(), item->GetFormID(), parentCellName, parentCellFormId);
        if (!filter.PassFilter(key.c_str())) continue;

        std::string displayName;
        if (wasDevMode) {
            displayName = std::format("{} 0x{:x}", item->GetDisplayFullName(), item->GetFormID());
        } else {
            displayName = std::format("{}", item->GetDisplayFullName());
        }

        if (totalCount[key] > 1) {
            int idx = ++runningIndex[key];
            displayName = std::format("{} #{}", displayName, idx);
        }

        std::string cellName;
        if (wasDevMode) {
            cellName = parentCellName ? std::format("{} 0x{:x}", parentCellName, parentCellFormId) : std::format("Unknown Cell 0x{:x}", parentCellFormId);
        } else {
            cellName = parentCellName ? std::format("{}", parentCellName) : std::format("Unknown Cell 0x{:x}", parentCellFormId);
        }

        cellGroups[cellName].push_back({item, displayName, globalIdx});
        globalIdx++;
    }

    std::vector<std::tuple<std::string, std::vector<NpcEntry>>> orderedGroups;

    for (auto& [cellName, entries] : cellGroups) {
        orderedGroups.push_back({cellName, std::move(entries)});
    }

    std::sort(orderedGroups.begin(), orderedGroups.end(), [](const auto& a, const auto& b) {
        return std::get<0>(a) < std::get<0>(b);
    });

    const int totalGroups = static_cast<int>(orderedGroups.size());
    const int totalPages = std::max(1, (totalGroups + pageSize - 1) / pageSize);
    if (currentPage >= totalPages) currentPage = totalPages - 1;
    if (currentPage < 0) currentPage = 0;

    const int groupStart = currentPage * pageSize;
    const int groupEnd = std::min(groupStart + pageSize, totalGroups);

    int columnCount = wasDevMode ? 4 : 3;
    int uid = 0;
    if (ImGuiMCP::BeginTable("NpcTable", columnCount, ImGuiMCP::ImGuiTableFlags_Borders | ImGuiMCP::ImGuiTableFlags_RowBg)) {
        ImGuiMCP::TableSetupColumn(Translation::Get("Window2.Table.Name"), ImGuiMCP::ImGuiTableColumnFlags_WidthFixed, 600.0f);
        ImGuiMCP::TableSetupColumn(Translation::Get("Window2.Table.Sex"), ImGuiMCP::ImGuiTableColumnFlags_WidthFixed, 256.0f);
        ImGuiMCP::TableSetupColumn(Translation::Get("Window2.Table.Level"), ImGuiMCP::ImGuiTableColumnFlags_WidthFixed, 256.0f);
        if (wasDevMode) {
            ImGuiMCP::TableSetupColumn(Translation::Get("DevMode.Window2.Table.Controls"), ImGuiMCP::ImGuiTableColumnFlags_WidthStretch);
        }
        ImGuiMCP::TableHeadersRow();

        for (int gi = groupStart; gi < groupEnd; gi++) {
            auto& [cellName, entries] = orderedGroups[gi];

            ImGuiMCP::TableNextRow();
            ImGuiMCP::TableSetColumnIndex(0);

            std::string groupLabel = std::format("{} ({})##grp_{}", cellName, entries.size(), cellName);
            bool groupOpen = ImGuiMCP::TreeNodeEx(groupLabel.c_str(), ImGuiMCP::ImGuiTreeNodeFlags_SpanAllColumns);

            if (!groupOpen) continue;

            for (auto& entry : entries) {
                auto item = entry.item;
                auto base = item->GetActorBase();

                ImGuiMCP::TableNextRow();
                ImGuiMCP::TableSetColumnIndex(0);

                if (!wasDevMode) {
                    if (ImGuiMCP::Selectable(std::format("{}##{:x}", entry.displayName, uid++).c_str(), false, ImGuiMCP::ImGuiSelectableFlags_SpanAllColumns)) {
                        Manager::SetBaseCharacter(item);
                        Next();
                    }
                } else {
                    ImGuiMCP::Text(entry.displayName.c_str());
                }

                ImGuiMCP::TableSetColumnIndex(1);
                bool isMale = base->GetSex() == RE::SEX::kMale;
                ImGuiMCP::ImVec4 sexColor = isMale ? ImGuiMCP::ImVec4{0.0f, 0.6f, 1.0f, 1.0f} : ImGuiMCP::ImVec4{1.0f, 0.4f, 0.8f, 1.0f};
                ImGuiMCP::TextColored(sexColor, "%s", isMale ? Translation::Get("Window2.Table.Male") : Translation::Get("Window2.Table.Female"));

                ImGuiMCP::TableSetColumnIndex(2);
                ImGuiMCP::Text("%d", base->GetLevel());
                
                if (wasDevMode) {
                    ImGuiMCP::TableSetColumnIndex(3);
                    if (ImGuiMCP::Button(std::format("{}##{:x}", Translation::Get("DevMode.Window2.Table.CopyId"), entry.item->GetFormID()).c_str())) {
                        FormID::Copy(entry.item);
                    }
                    ImGuiMCP::SameLine();
                    if (ImGuiMCP::Button(std::format("{}##{:x}", Translation::Get("DevMode.Window2.Table.CopyBaseId"), entry.item->GetFormID()).c_str())) {
                        FormID::Copy(entry.item->GetBaseObject());
                    }
                    ImGuiMCP::SameLine();
                    if (ImGuiMCP::Button(std::format("{}##{:x}", Translation::Get("DevMode.Window2.Table.CopyCellId"), entry.item->GetFormID()).c_str())) {
                        FormID::Copy(entry.item->GetParentCell());
                    }
                    ImGuiMCP::SameLine();
                    if (ImGuiMCP::Button(std::format("{}##{:x}", Translation::Get("DevMode.Window2.Table.ChooseCharacter"), entry.item->GetFormID()).c_str())) {
                        Manager::SetBaseCharacter(item);
                        Next();
                    }
                }
            }

            ImGuiMCP::TreePop();
        }

        ImGuiMCP::EndTable();
    }

    // Pagination controls
    ImGuiMCP::Separator();

    FontAwesome::PushSolid();

    ImGuiMCP::BeginDisabled(currentPage <= 0);
    if (ImGuiMCP::Button(Translation::Get("Window2.Table.Previus"))) currentPage--;
    ImGuiMCP::EndDisabled();

    ImGuiMCP::SameLine();
    ImGuiMCP::Text(std::format("{} {} / {}", Translation::Get("Window2.Table.Page"), currentPage + 1, totalPages).c_str());
    ImGuiMCP::SameLine();

    ImGuiMCP::BeginDisabled(currentPage >= totalPages - 1);
    if (ImGuiMCP::Button(Translation::Get("Window2.Table.Next"))) currentPage++;
    ImGuiMCP::EndDisabled();
    FontAwesome::Pop();

    ImGuiMCP::End();
}



void __stdcall UI::NewGame::RenderChooseSideWindow() {
    BeguinWindow(Translation::Get("Window3.Settings"));

    ImGuiMCP::ImVec2 region;
    ImGuiMCP::GetContentRegionAvail(&region);

    float contentWidth = region.x * 0.7f;
    float contentHeight = region.y * 0.7f;
    float offsetX = (region.x - contentWidth) * 0.5f;

    float itemSpacing = ImGuiMCP::GetStyle()->ItemSpacing.y;
    float frameHeight = ImGuiMCP::GetFrameHeight();
    float toggleHeight = frameHeight;  // approximate height of a ToggleButton row

    float totalHeight = toggleHeight + itemSpacing +  // ToggleButton 1
                        toggleHeight + itemSpacing +  // ToggleButton 2
                        itemSpacing +                 // Spacing
                        1.0f + itemSpacing +          // Separator
                        itemSpacing +                 // Spacing
                        50.0f + itemSpacing +         // Next button
                        50.0f;                        // Go Back button

    float offsetY = (region.y - totalHeight) * 0.5f;
    if (offsetY > 0.0f) ImGuiMCP::SetCursorPosY(ImGuiMCP::GetCursorPosY() + offsetY);

    ImGuiMCP::SetCursorPosX(offsetX);
    ImGuiMCP::BeginGroup();


    if (Manager::defaultStart) {
        Manager::startMainQuestLine = true;
        Manager::startAtNPCLocation = false;
    }

     ImGuiMCPComponents::ToggleButton(Translation::Get("Window3.DefaultStart"), &Manager::defaultStart);


    ImGuiMCPComponents::ToggleButton(Translation::Get("Window3.StartMainQuestline"), &Manager::startMainQuestLine, Manager::defaultStart);
     ImGuiMCPComponents::ToggleButton(Translation::Get("Window3.StartAtNpcLocation"), &Manager::startAtNPCLocation, Manager::defaultStart);



    ImGuiMCP::Spacing();
    ImGuiMCP::Separator();
    ImGuiMCP::Spacing();

    ImGuiMCP::Spacing();
    ImGuiMCP::Separator();
    ImGuiMCP::Spacing();

    float spacingX = ImGuiMCP::GetStyle()->ItemSpacing.x;
    float halfWidth = (contentWidth - spacingX) * 0.5f;

    if (ImGuiMCP::Button(Translation::Get("Controls.GoBack"), ImGuiMCP::ImVec2(halfWidth, 50.0f))) Previus();

    ImGuiMCP::SameLine();

    if (ImGuiMCP::Button(Translation::Get("Controls.Next"), ImGuiMCP::ImVec2(halfWidth, 50.0f))) Next();

    ImGuiMCP::EndGroup();
    ImGuiMCP::End();
}

void __stdcall UI::NewGame::RenderReviewWindow() {
    BeguinWindow(Translation::Get("Window4.ChooseAndReview"));

    ImGuiMCP::ImVec2 region;
    ImGuiMCP::GetContentRegionAvail(&region);

    float contentWidth = region.x * 0.7f;
    float offsetX = (region.x - contentWidth) * 0.5f;

    float totalHeight = ImGuiMCP::GetTextLineHeightWithSpacing() + ImGuiMCP::GetStyle()->ItemSpacing.y + ImGuiMCP::GetTextLineHeightWithSpacing() + ImGuiMCP::GetStyle()->ItemSpacing.y + ImGuiMCP::GetTextLineHeightWithSpacing() +
                        ImGuiMCP::GetStyle()->ItemSpacing.y + 50.0f + ImGuiMCP::GetStyle()->ItemSpacing.y + 50.0f;

    float offsetY = (region.y - totalHeight) * 0.5f + 25.f;
    if (offsetY > 0.0f) ImGuiMCP::SetCursorPosY(offsetY);

    ImGuiMCP::SetCursorPosX(offsetX);
    ImGuiMCP::BeginGroup();



    const char* baseName = Manager::GetBaseCharacter() ? Manager::GetBaseCharacter()->GetDisplayFullName() : "Unknown";

    ImGuiMCP::Text(Translation::Get("Window4.BaseCharacter"));
    ImGuiMCP::BulletText("%s", baseName);
    ImGuiMCP::Spacing();

    if (!Manager::defaultStart) {
        ImGuiMCP::Text(Translation::Get("Window4.StartMainQuestline"));
        if (Manager::startMainQuestLine) {
            ImGuiMCP::BulletText("%s", Translation::Get("Controls.Yes"));
        } else {
            ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
            ImGuiMCP::BulletText("%s", Translation::Get("Window4.StartMainQuestlineNo"));
            ImGuiMCP::PopStyleColor();
        }
        ImGuiMCP::Spacing();
        ImGuiMCP::Text(Translation::Get("Window4.StartAtNpcLocation"));


        if (Manager::startAtNPCLocation) {
            ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
            ImGuiMCP::BulletText("%s", Translation::Get("Window4.StartAtNpcLocationYes"));
            ImGuiMCP::PopStyleColor();
        } else {
            ImGuiMCP::BulletText("%s", Translation::Get("Controls.No"));
        }

        ImGuiMCP::Spacing();
    }
    ImGuiMCP::Text(Translation::Get("Window4.DefaultStart"));

    if (Manager::defaultStart) {
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
        ImGuiMCP::BulletText("%s", Translation::Get("Window4.DefaultStartYes"));
        ImGuiMCP::PopStyleColor();
    } else {
        ImGuiMCP::BulletText("%s", Translation::Get("Controls.No"));
    }

    ImGuiMCP::Spacing();
    ImGuiMCP::Separator();
    ImGuiMCP::Spacing();

    ImGuiMCP::ImVec2 halfButton = {contentWidth * 0.5f - 4.0f, 50.0f};

    if (ImGuiMCP::Button(Translation::Get("Controls.GoBack"), halfButton)) {
        Previus();
    }

    ImGuiMCP::SameLine();

    ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Button, ImGuiMCP::ImVec4(0.1f, 0.6f, 0.1f, 1.0f));
    ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_ButtonHovered, ImGuiMCP::ImVec4(0.1f, 0.8f, 0.1f, 1.0f));
    ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_ButtonActive, ImGuiMCP::ImVec4(0.2f, 0.9f, 0.2f, 1.0f));

    if (ImGuiMCP::Button(Translation::Get("Controls.Finish"), halfButton)) {
        Next();
    }

    ImGuiMCP::PopStyleColor(3);


    ImGuiMCP::EndGroup();
    ImGuiMCP::End();
}



void __stdcall UI::NewGame::RenderBackground() {
    if (index == 0) {
        return;
    }

    auto texture = SKSEMenuFramework::LoadTexture(".\\Data\\textures\\interface\\SkyrimThiago\\menu-bg.dds");
    auto srv = reinterpret_cast<ID3D11ShaderResourceView*>(texture);

    auto drawList = ImGuiMCP::GetBackgroundDrawList();
    auto viewport = ImGuiMCP::GetMainViewport();
    ImGuiMCP::ImVec2 screenMin = viewport->Pos;
    ImGuiMCP::ImVec2 screenSize = viewport->Size;

    // Get texture size from D3D resource
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ID3D11Resource* resource = nullptr;
    srv->GetResource(&resource);
    ID3D11Texture2D* tex2D = nullptr;
    resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&tex2D));
    D3D11_TEXTURE2D_DESC texDesc;
    tex2D->GetDesc(&texDesc);
    float imgWidth = static_cast<float>(texDesc.Width);
    float imgHeight = static_cast<float>(texDesc.Height);
    tex2D->Release();
    resource->Release();

    float scaleX = screenSize.x / imgWidth;
    float scaleY = screenSize.y / imgHeight;
    float scale = std::max(scaleX, scaleY);

    float drawWidth = imgWidth * scale;
    float drawHeight = imgHeight * scale;

    ImGuiMCP::ImVec2 drawMin = ImGuiMCP::ImVec2(screenMin.x + (screenSize.x - drawWidth) / 2.0f, screenMin.y + (screenSize.y - drawHeight) / 2.0f);
    ImGuiMCP::ImVec2 drawMax = ImGuiMCP::ImVec2(drawMin.x + drawWidth, drawMin.y + drawHeight);

    ImGuiMCP::ImDrawListManager::AddImage(drawList, texture, drawMin, drawMax, {0, 0}, {1, 1}, IM_COL32(255, 255, 255, 255));
}
