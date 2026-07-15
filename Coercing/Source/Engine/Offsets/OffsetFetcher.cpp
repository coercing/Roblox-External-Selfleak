#include "OffsetFetcher.h"
#include "Offsets.h"
#include "Miscellaneous/Output/Output.hpp"
#include "Driver/Driver.h"
#include <Windows.h>
#include <winhttp.h>
#include <vector>
#include <string>
#include <cctype>

#pragma comment(lib, "winhttp.lib")

namespace OffsetFetcher {

    std::string RobloxVersion;

    static std::string FetchURL(const std::wstring& host, const std::wstring& path, bool secure) {
        std::string response;
        HINTERNET hSession = WinHttpOpen(L"OffsetFetcher/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
        if (!hSession) return response;

        HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), secure ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT, 0);
        if (!hConnect) { WinHttpCloseHandle(hSession); return response; }

        DWORD flags = secure ? WINHTTP_FLAG_SECURE : 0;
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
        if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return response; }

        BOOL bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)
            && WinHttpReceiveResponse(hRequest, NULL);

        if (bResults) {
            DWORD dwSize = 0;
            do {
                DWORD dwDownloaded = 0;
                if (!WinHttpQueryDataAvailable(hRequest, &dwSize) || !dwSize) break;
                std::string buffer(dwSize, 0);
                if (!WinHttpReadData(hRequest, &buffer[0], dwSize, &dwDownloaded)) break;
                response.append(buffer, 0, dwDownloaded);
            } while (dwSize > 0);
        }

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return response;
    }

    struct JsonValue {
        std::string str;
        uintptr_t num = 0;
        bool isNumber = false;
        bool isString = false;
        bool isObject = false;
        std::vector<std::pair<std::string, JsonValue>> fields;
    };

    static void SkipWS(const std::string& j, size_t& p) {
        while (p < j.size() && (j[p] == ' ' || j[p] == '\t' || j[p] == '\n' || j[p] == '\r')) p++;
    }

    static JsonValue ParseValue(const std::string& j, size_t& p) {
        JsonValue v;
        SkipWS(j, p);
        if (p >= j.size()) return v;

        if (j[p] == '{') {
            v.isObject = true;
            p++;
            while (p < j.size() && j[p] != '}') {
                SkipWS(j, p);
                if (j[p] == '}') break;
                if (j[p] != '"') { p++; continue; }
                size_t ks = p + 1, ke = j.find('"', ks);
                std::string key = j.substr(ks, ke - ks);
                p = ke + 1;
                SkipWS(j, p);
                if (j[p] == ':') p++;
                SkipWS(j, p);
                v.fields.emplace_back(key, ParseValue(j, p));
                SkipWS(j, p);
                if (j[p] == ',') p++;
            }
            if (p < j.size() && j[p] == '}') p++;
        }
        else if (j[p] == '"') {
            v.isString = true;
            size_t s = p + 1, e = j.find('"', s);
            v.str = j.substr(s, e - s);
            p = e + 1;
        }
        else if (j[p] == '[') {
            int depth = 1; p++;
            while (p < j.size() && depth > 0) { if (j[p] == '[') depth++; else if (j[p] == ']') depth--; p++; }
        }
        else {
            v.isNumber = true;
            size_t s = p;
            while (p < j.size() && j[p] != ',' && j[p] != '}' && j[p] != ']' && !isspace((unsigned char)j[p])) p++;
            std::string ns = j.substr(s, p - s);
            if (ns.size() > 2 && ns[0] == '0' && (ns[1] == 'x' || ns[1] == 'X'))
                v.num = std::stoull(ns.substr(2), nullptr, 16);
            else
                v.num = std::stoull(ns);
        }
        return v;
    }

#define SET_PROP(ns_name, field_name) \
    if (prop == #field_name) { Offsets::ns_name::field_name = val; return; }

    static void SetHumanoid(const std::string& prop, uintptr_t val) {
        SET_PROP(Humanoid, AutoJumpEnabled) else
        SET_PROP(Humanoid, AutoRotate) else
        SET_PROP(Humanoid, BreakJointsOnDeath) else
        SET_PROP(Humanoid, CameraOffset) else
        SET_PROP(Humanoid, DisplayDistanceType) else
        SET_PROP(Humanoid, DisplayName) else
        SET_PROP(Humanoid, EvaluateStateMachine) else
        SET_PROP(Humanoid, FloorMaterial) else
        SET_PROP(Humanoid, Health) else
        SET_PROP(Humanoid, HealthDisplayDistance) else
        SET_PROP(Humanoid, HealthDisplayType) else
        SET_PROP(Humanoid, HipHeight) else
        SET_PROP(Humanoid, HumanoidRootPart) else
        SET_PROP(Humanoid, HumanoidState) else
        SET_PROP(Humanoid, HumanoidStateID) else
        SET_PROP(Humanoid, IsWalking) else
        SET_PROP(Humanoid, Jump) else
        SET_PROP(Humanoid, JumpHeight) else
        SET_PROP(Humanoid, JumpPower) else
        SET_PROP(Humanoid, MaxHealth) else
        SET_PROP(Humanoid, MaxSlopeAngle) else
        SET_PROP(Humanoid, MoveDirection) else
        SET_PROP(Humanoid, MoveToPart) else
        SET_PROP(Humanoid, MoveToPoint) else
        SET_PROP(Humanoid, NameDisplayDistance) else
        SET_PROP(Humanoid, NameOcclusion) else
        SET_PROP(Humanoid, PlatformStand) else
        SET_PROP(Humanoid, RequiresNeck) else
        SET_PROP(Humanoid, RigType) else
        SET_PROP(Humanoid, SeatPart) else
        SET_PROP(Humanoid, Sit) else
        SET_PROP(Humanoid, TargetPoint) else
        SET_PROP(Humanoid, Walkspeed) else
        SET_PROP(Humanoid, WalkspeedCheck)
    }

    static void SetCamera(const std::string& prop, uintptr_t val) {
        SET_PROP(Camera, CameraSubject) else
        SET_PROP(Camera, CameraType) else
        SET_PROP(Camera, FieldOfView) else
        SET_PROP(Camera, ImagePlaneDepth) else
        SET_PROP(Camera, Position) else
        SET_PROP(Camera, RaycastController) else
        SET_PROP(Camera, RaycastController2) else
        SET_PROP(Camera, Rotation) else
        SET_PROP(Camera, Viewport) else
        SET_PROP(Camera, ViewportSize)
    }

    static void SetPlayer(const std::string& prop, uintptr_t val) {
        SET_PROP(Player, AccountAge) else
        SET_PROP(Player, CameraMode) else
        SET_PROP(Player, DisplayName) else
        SET_PROP(Player, HealthDisplayDistance) else
        SET_PROP(Player, LocalPlayer) else
        SET_PROP(Player, LocaleId) else
        SET_PROP(Player, MaxZoomDistance) else
        SET_PROP(Player, MinZoomDistance) else
        SET_PROP(Player, ModelInstance) else
        SET_PROP(Player, Mouse) else
        SET_PROP(Player, NameDisplayDistance) else
        SET_PROP(Player, Team) else
        SET_PROP(Player, TeamColor) else
        SET_PROP(Player, UserId)
    }

    static void SetLighting(const std::string& prop, uintptr_t val) {
        SET_PROP(Lighting, Ambient) else
        SET_PROP(Lighting, Brightness) else
        SET_PROP(Lighting, ClockTime) else
        SET_PROP(Lighting, ColorShift_Bottom) else
        SET_PROP(Lighting, ColorShift_Top) else
        SET_PROP(Lighting, EnvironmentDiffuseScale) else
        SET_PROP(Lighting, EnvironmentSpecularScale) else
        SET_PROP(Lighting, ExposureCompensation) else
        SET_PROP(Lighting, FogColor) else
        SET_PROP(Lighting, FogEnd) else
        SET_PROP(Lighting, FogStart) else
        SET_PROP(Lighting, GeographicLatitude) else
        SET_PROP(Lighting, GlobalShadows) else
        SET_PROP(Lighting, GradientBottom) else
        SET_PROP(Lighting, GradientTop) else
        SET_PROP(Lighting, LightColor) else
        SET_PROP(Lighting, LightDirection) else
        SET_PROP(Lighting, MoonPosition) else
        SET_PROP(Lighting, OutdoorAmbient) else
        SET_PROP(Lighting, Source) else
        SET_PROP(Lighting, SunPosition)
    }

    static void SetBasePart(const std::string& prop, uintptr_t val) {
        SET_PROP(BasePart, CastShadow) else
        SET_PROP(BasePart, Color3) else
        SET_PROP(BasePart, Locked) else
        SET_PROP(BasePart, Massless) else
        SET_PROP(BasePart, Primitive) else
        SET_PROP(BasePart, RaycastListener) else
        SET_PROP(BasePart, Reflectance) else
        SET_PROP(BasePart, Shape) else
        SET_PROP(BasePart, Transparency)
    }

    static void SetPrimitive(const std::string& prop, uintptr_t val) {
        SET_PROP(Primitive, AssemblyAngularVelocity) else
        SET_PROP(Primitive, AssemblyLinearVelocity) else
        SET_PROP(Primitive, Flags) else
        SET_PROP(Primitive, Material) else
        SET_PROP(Primitive, Owner) else
        SET_PROP(Primitive, Position) else
        SET_PROP(Primitive, Rotation) else
        SET_PROP(Primitive, Size) else
        SET_PROP(Primitive, Validate)
    }

    static void SetGuiObject(const std::string& prop, uintptr_t val) {
        SET_PROP(GuiObject, BackgroundColor3) else
        SET_PROP(GuiObject, BackgroundTransparency) else
        SET_PROP(GuiObject, BorderColor3) else
        SET_PROP(GuiObject, Image) else
        SET_PROP(GuiObject, LayoutOrder) else
        SET_PROP(GuiObject, Position) else
        SET_PROP(GuiObject, RichText) else
        SET_PROP(GuiObject, Rotation) else
        SET_PROP(GuiObject, ScreenGui_Enabled) else
        SET_PROP(GuiObject, Size) else
        SET_PROP(GuiObject, Text) else
        SET_PROP(GuiObject, TextColor3) else
        SET_PROP(GuiObject, Visible) else
        SET_PROP(GuiObject, ZIndex)
    }

    static void SetProximityPrompt(const std::string& prop, uintptr_t val) {
        SET_PROP(ProximityPrompt, ActionText) else
        SET_PROP(ProximityPrompt, Enabled) else
        SET_PROP(ProximityPrompt, GamepadKeyCode) else
        SET_PROP(ProximityPrompt, HoldDuration) else
        SET_PROP(ProximityPrompt, KeyCode) else
        SET_PROP(ProximityPrompt, MaxActivationDistance) else
        SET_PROP(ProximityPrompt, ObjectText) else
        SET_PROP(ProximityPrompt, RequiresLineOfSight)
    }

    static void SetOffset(const std::string& ns, const std::string& prop, uintptr_t val) {
        if (ns == "Humanoid") { SetHumanoid(prop, val); return; }
        if (ns == "Camera") { SetCamera(prop, val); return; }
        if (ns == "Player") { SetPlayer(prop, val); return; }
        if (ns == "Lighting") { SetLighting(prop, val); return; }
        if (ns == "BasePart") { SetBasePart(prop, val); return; }
        if (ns == "Primitive") { SetPrimitive(prop, val); return; }
        if (ns == "GuiObject") { SetGuiObject(prop, val); return; }
        if (ns == "ProximityPrompt") { SetProximityPrompt(prop, val); return; }
        if (ns == "RaycastController") { SET_PROP(RaycastController, Ray) }
        if (ns == "RaycastListener") { SET_PROP(RaycastListener, Ray) }
        if (ns == "RVA") { SET_PROP(RVA, RaycastUpdater) }
        if (ns == "VTable") { SET_PROP(VTable, RayControllerPost) }

        // Smaller / single-field namespaces:
        if (ns == "AirProperties") {
            SET_PROP(AirProperties, AirDensity) else
            SET_PROP(AirProperties, GlobalWind)
        }
        if (ns == "AnimationTrack") {
            SET_PROP(AnimationTrack, Animation) else
            SET_PROP(AnimationTrack, Animator) else
            SET_PROP(AnimationTrack, IsPlaying) else
            SET_PROP(AnimationTrack, Looped) else
            SET_PROP(AnimationTrack, Speed) else
            SET_PROP(AnimationTrack, TimePosition)
        }
        if (ns == "Animator") { SET_PROP(Animator, ActiveAnimations) }
        if (ns == "Atmosphere") {
            SET_PROP(Atmosphere, Color) else
            SET_PROP(Atmosphere, Decay) else
            SET_PROP(Atmosphere, Density) else
            SET_PROP(Atmosphere, Glare) else
            SET_PROP(Atmosphere, Haze) else
            SET_PROP(Atmosphere, Offset)
        }
        if (ns == "Attachment") { SET_PROP(Attachment, Position) }
        if (ns == "BloomEffect") {
            SET_PROP(BloomEffect, Enabled) else
            SET_PROP(BloomEffect, Intensity) else
            SET_PROP(BloomEffect, Size) else
            SET_PROP(BloomEffect, Threshold)
        }
        if (ns == "BlurEffect") {
            SET_PROP(BlurEffect, Enabled) else
            SET_PROP(BlurEffect, Size)
        }
        if (ns == "ByteCode") {
            SET_PROP(ByteCode, Pointer) else
            SET_PROP(ByteCode, Size)
        }
        if (ns == "CharacterMesh") {
            SET_PROP(CharacterMesh, BaseTextureId) else
            SET_PROP(CharacterMesh, BodyPart) else
            SET_PROP(CharacterMesh, MeshId) else
            SET_PROP(CharacterMesh, OverlayTextureId)
        }
        if (ns == "ClickDetector") {
            SET_PROP(ClickDetector, MaxActivationDistance) else
            SET_PROP(ClickDetector, MouseIcon)
        }
        if (ns == "Clothing") {
            SET_PROP(Clothing, Color3) else
            SET_PROP(Clothing, Template)
        }
        if (ns == "ColorCorrectionEffect") {
            SET_PROP(ColorCorrectionEffect, Brightness) else
            SET_PROP(ColorCorrectionEffect, Contrast) else
            SET_PROP(ColorCorrectionEffect, Enabled) else
            SET_PROP(ColorCorrectionEffect, TintColor)
        }
        if (ns == "ColorGradingEffect") {
            SET_PROP(ColorGradingEffect, Enabled) else
            SET_PROP(ColorGradingEffect, TonemapperPreset)
        }
        if (ns == "DataModel") {
            SET_PROP(DataModel, CreatorId) else
            SET_PROP(DataModel, GameId) else
            SET_PROP(DataModel, GameLoaded) else
            SET_PROP(DataModel, JobId) else
            SET_PROP(DataModel, PlaceId) else
            SET_PROP(DataModel, PlaceVersion) else
            SET_PROP(DataModel, PrimitiveCount) else
            SET_PROP(DataModel, ScriptContext) else
            SET_PROP(DataModel, ServerIP) else
            SET_PROP(DataModel, ToRenderView1) else
            SET_PROP(DataModel, ToRenderView2) else
            SET_PROP(DataModel, ToRenderView3) else
            SET_PROP(DataModel, Workspace)
        }
        if (ns == "DepthOfFieldEffect") {
            SET_PROP(DepthOfFieldEffect, Enabled) else
            SET_PROP(DepthOfFieldEffect, FarIntensity) else
            SET_PROP(DepthOfFieldEffect, FocusDistance) else
            SET_PROP(DepthOfFieldEffect, InFocusRadius) else
            SET_PROP(DepthOfFieldEffect, NearIntensity)
        }
        if (ns == "DragDetector") {
            SET_PROP(DragDetector, ActivatedCursorIcon) else
            SET_PROP(DragDetector, CursorIcon) else
            SET_PROP(DragDetector, MaxActivationDistance)
        }
        if (ns == "FakeDataModel") {
            SET_PROP(FakeDataModel, Pointer) else
            SET_PROP(FakeDataModel, RealDataModel)
        }
        if (ns == "GuiBase2D") {
            SET_PROP(GuiBase2D, AbsolutePosition) else
            SET_PROP(GuiBase2D, AbsoluteRotation) else
            SET_PROP(GuiBase2D, AbsoluteSize)
        }
        if (ns == "Instance") {
            SET_PROP(Instance, ClassDescriptor) else
            SET_PROP(Instance, ClassName) else
            SET_PROP(Instance, Name) else
            SET_PROP(Instance, Parent)
        }
        if (ns == "LocalScript") {
            SET_PROP(LocalScript, ByteCode) else
            SET_PROP(LocalScript, GUID) else
            SET_PROP(LocalScript, Hash)
        }
        if (ns == "MeshPart") {
            SET_PROP(MeshPart, MeshId) else
            SET_PROP(MeshPart, Texture) else
            SET_PROP(MeshPart, Color3)
        }
        if (ns == "Misc") {
            SET_PROP(Misc, Adornee) else
            SET_PROP(Misc, AnimationId) else
            SET_PROP(Misc, StringLength) else
            SET_PROP(Misc, Value)
        }
        if (ns == "Model") {
            SET_PROP(Model, PrimaryPart) else
            SET_PROP(Model, Scale)
        }
        if (ns == "ModuleScript") {
            SET_PROP(ModuleScript, ByteCode) else
            SET_PROP(ModuleScript, GUID) else
            SET_PROP(ModuleScript, Hash)
        }
        if (ns == "MouseService") {
            SET_PROP(MouseService, InputObject) else
            SET_PROP(MouseService, MousePosition) else
            SET_PROP(MouseService, SensitivityPointer)
        }
        if (ns == "PlayerConfigurer") { SET_PROP(PlayerConfigurer, Pointer) }
        if (ns == "PlayerMouse") {
            SET_PROP(PlayerMouse, Icon) else
            SET_PROP(PlayerMouse, Workspace)
        }
        if (ns == "PrimitiveFlags") {
            SET_PROP(PrimitiveFlags, Anchored) else
            SET_PROP(PrimitiveFlags, CanCollide) else
            SET_PROP(PrimitiveFlags, CanQuery) else
            SET_PROP(PrimitiveFlags, CanTouch)
        }
        if (ns == "RenderJob") {
            SET_PROP(RenderJob, FakeDataModel) else
            SET_PROP(RenderJob, RealDataModel) else
            SET_PROP(RenderJob, RenderView)
        }
        if (ns == "RenderView") {
            SET_PROP(RenderView, DeviceD3D11) else
            SET_PROP(RenderView, LightingValid) else
            SET_PROP(RenderView, VisualEngine)
        }
        if (ns == "RunService") {
            SET_PROP(RunService, HeartbeatFPS) else
            SET_PROP(RunService, HeartbeatTask)
        }
        if (ns == "Seat") { SET_PROP(Seat, Occupant) }
        if (ns == "Sound") {
            SET_PROP(Sound, Looped) else
            SET_PROP(Sound, PlaybackSpeed) else
            SET_PROP(Sound, RollOffMaxDistance) else
            SET_PROP(Sound, RollOffMinDistance) else
            SET_PROP(Sound, SoundGroup) else
            SET_PROP(Sound, SoundId) else
            SET_PROP(Sound, Volume)
        }
        if (ns == "SpawnLocation") {
            SET_PROP(SpawnLocation, AllowTeamChangeOnTouch) else
            SET_PROP(SpawnLocation, Enabled) else
            SET_PROP(SpawnLocation, ForcefieldDuration) else
            SET_PROP(SpawnLocation, Neutral) else
            SET_PROP(SpawnLocation, TeamColor)
        }
        if (ns == "SpecialMesh") {
            SET_PROP(SpecialMesh, MeshId) else
            SET_PROP(SpecialMesh, Scale)
        }
        if (ns == "StatsItem") { SET_PROP(StatsItem, Value) }
        if (ns == "SunRaysEffect") {
            SET_PROP(SunRaysEffect, Enabled) else
            SET_PROP(SunRaysEffect, Intensity) else
            SET_PROP(SunRaysEffect, Spread)
        }
        if (ns == "TaskScheduler") {
            SET_PROP(TaskScheduler, JobEnd) else
            SET_PROP(TaskScheduler, JobName) else
            SET_PROP(TaskScheduler, JobStart) else
            SET_PROP(TaskScheduler, MaxFPS) else
            SET_PROP(TaskScheduler, Pointer)
        }
        if (ns == "Team") { SET_PROP(Team, BrickColor) }
        if (ns == "Terrain") {
            SET_PROP(Terrain, GrassLength) else
            SET_PROP(Terrain, MaterialColors) else
            SET_PROP(Terrain, WaterColor) else
            SET_PROP(Terrain, WaterReflectance) else
            SET_PROP(Terrain, WaterTransparency) else
            SET_PROP(Terrain, WaterWaveSize) else
            SET_PROP(Terrain, WaterWaveSpeed)
        }
        if (ns == "Textures") {
            SET_PROP(Textures, Decal_Texture) else
            SET_PROP(Textures, Texture_Texture)
        }
        if (ns == "Tool") {
            SET_PROP(Tool, CanBeDropped) else
            SET_PROP(Tool, Enabled) else
            SET_PROP(Tool, Grip) else
            SET_PROP(Tool, ManualActivationOnly) else
            SET_PROP(Tool, RequiresHandle) else
            SET_PROP(Tool, TextureId) else
            SET_PROP(Tool, Tooltip)
        }
        if (ns == "VisualEngine") {
            SET_PROP(VisualEngine, Dimensions) else
            SET_PROP(VisualEngine, FakeDataModel) else
            SET_PROP(VisualEngine, Pointer) else
            SET_PROP(VisualEngine, RenderView) else
            SET_PROP(VisualEngine, ViewMatrix)
        }
        if (ns == "Workspace") {
            SET_PROP(Workspace, CurrentCamera) else
            SET_PROP(Workspace, DistributedGameTime) else
            SET_PROP(Workspace, ReadOnlyGravity) else
            SET_PROP(Workspace, World)
        }
        if (ns == "World") {
            SET_PROP(World, AirProperties) else
            SET_PROP(World, FallenPartsDestroyHeight) else
            SET_PROP(World, Gravity) else
            SET_PROP(World, Primitives) else
            SET_PROP(World, worldStepsPerSec)
        }
    }

#undef SET_PROP

    static void ApplyNested(const JsonValue& root) {
        const JsonValue* offsetsObj = nullptr;
        for (auto& f : root.fields) {
            if (f.first == "Offsets" && f.second.isObject) {
                offsetsObj = &f.second;
                break;
            }
        }
        if (!offsetsObj) return;

        for (auto& nsField : offsetsObj->fields) {
            if (!nsField.second.isObject) continue;
            for (auto& propField : nsField.second.fields) {
                if (!propField.second.isNumber) continue;
                SetOffset(nsField.first, propField.first, propField.second.num);
            }
        }
    }

    static void ApplyFlat(const JsonValue& root) {
        if (root.fields.empty()) return;

        for (auto& f : root.fields) {
            if (f.first == "RobloxVersion" && f.second.isString) {
                RobloxVersion = f.second.str;
                break;
            }
        }

#define FLAT(flat_name, ns, field) \
    if (k == #flat_name) { Offsets::ns::field = v; }

        for (auto& f : root.fields) {
            if (!f.second.isNumber) continue;
            const std::string& k = f.first;
            uintptr_t v = f.second.num;

            if (false) {}
            FLAT(Adornee, Misc, Adornee)
            else FLAT(Anchored, PrimitiveFlags, Anchored)
            else FLAT(AnimationId, Misc, AnimationId)
            else FLAT(AutoJumpEnabled, Humanoid, AutoJumpEnabled)
            else FLAT(Camera, Workspace, CurrentCamera)
            else FLAT(CameraMaxZoomDistance, Player, MaxZoomDistance)
            else FLAT(CameraMinZoomDistance, Player, MinZoomDistance)
            else FLAT(CameraMode, Player, CameraMode)
            else FLAT(CameraPos, Camera, Position)
            else FLAT(CameraRotation, Camera, Rotation)
            else FLAT(CameraSubject, Camera, CameraSubject)
            else FLAT(CameraType, Camera, CameraType)
            else FLAT(CanCollide, PrimitiveFlags, CanCollide)
            else FLAT(CanTouch, PrimitiveFlags, CanTouch)
            else FLAT(ChildrenEnd, Instance, Parent)
            else FLAT(ClassDescriptor, Instance, ClassDescriptor)
            else FLAT(ClockTime, Lighting, ClockTime)
            else FLAT(CreatorId, DataModel, CreatorId)
            else FLAT(DataModelPrimitiveCount, DataModel, PrimitiveCount)
            else FLAT(DecalTexture, Textures, Decal_Texture)
            else FLAT(DisplayName, Player, DisplayName)
            else FLAT(FOV, Camera, FieldOfView)
            else FLAT(FakeDataModelPointer, FakeDataModel, Pointer)
            else FLAT(FakeDataModelToDataModel, FakeDataModel, RealDataModel)
            else FLAT(FogColor, Lighting, FogColor)
            else FLAT(FogEnd, Lighting, FogEnd)
            else FLAT(FogStart, Lighting, FogStart)
            else FLAT(GameId, DataModel, GameId)
            else FLAT(GameLoaded, DataModel, GameLoaded)
            else FLAT(Gravity, World, Gravity)
            else FLAT(Health, Humanoid, Health)
            else FLAT(HealthDisplayDistance, Player, HealthDisplayDistance)
            else FLAT(HipHeight, Humanoid, HipHeight)
            else FLAT(HumanoidDisplayName, Humanoid, DisplayName)
            else FLAT(HumanoidState, Humanoid, HumanoidState)
            else FLAT(HumanoidStateId, Humanoid, HumanoidStateID)
            else FLAT(InputObject, MouseService, InputObject)
            else FLAT(JobEnd, TaskScheduler, JobEnd)
            else FLAT(JobId, DataModel, JobId)
            else FLAT(JobStart, TaskScheduler, JobStart)
            else FLAT(Job_Name, TaskScheduler, JobName)
            else FLAT(JobsPointer, TaskScheduler, Pointer)
            else FLAT(JumpPower, Humanoid, JumpPower)
            else FLAT(LocalPlayer, Player, LocalPlayer)
            else FLAT(MaterialType, Primitive, MaterialType)
            else FLAT(MaxHealth, Humanoid, MaxHealth)
            else FLAT(MaxSlopeAngle, Humanoid, MaxSlopeAngle)
            else FLAT(MeshPartColor3, MeshPart, Color3)
            else FLAT(MeshPartTexture, MeshPart, Texture)
            else FLAT(ModelInstance, Player, ModelInstance)
            else FLAT(MousePosition, MouseService, MousePosition)
            else FLAT(MouseSensitivity, MouseService, SensitivityPointer)
            else FLAT(MoveDirection, Humanoid, MoveDirection)
            else FLAT(Name, Instance, Name)
            else FLAT(NameDisplayDistance, Player, NameDisplayDistance)
            else FLAT(OutdoorAmbient, Lighting, OutdoorAmbient)
            else FLAT(Parent, Instance, Parent)
            else FLAT(Ping, StatsItem, Value)
            else FLAT(PlaceId, DataModel, PlaceId)
            else FLAT(PlayerConfigurerPointer, PlayerConfigurer, Pointer)
            else FLAT(PlayerMouse, Player, Mouse)
            else FLAT(Position, Primitive, Position)
            else FLAT(Primitive, BasePart, Primitive)
            else             FLAT(PrimitiveValidateValue, Primitive, Validate)
            else FLAT(PrimitivesPointer1, World, Primitives)
            else FLAT(RaycastController, Camera, RaycastController)
            else FLAT(RaycastController2, Camera, RaycastController2)
            else FLAT(RaycastListener, BasePart, RaycastListener)
            else FLAT(RaycastControllerRay, RaycastController, Ray)
            else FLAT(RaycastListenerRay, RaycastListener, Ray)
            else FLAT(RaycastUpdaterRVA, RVA, RaycastUpdater)
            else FLAT(VTableRayControllerPost, VTable, RayControllerPost)
            else FLAT(ProximityPromptActionText, ProximityPrompt, ActionText)
            else FLAT(ProximityPromptEnabled, ProximityPrompt, Enabled)
            else FLAT(ProximityPromptGamepadKeyCode, ProximityPrompt, GamepadKeyCode)
            else FLAT(ProximityPromptHoldDuraction, ProximityPrompt, HoldDuration)
            else FLAT(ProximityPromptMaxActivationDistance, ProximityPrompt, MaxActivationDistance)
            else FLAT(RenderJobToDataModel, RenderJob, RealDataModel)
            else FLAT(RenderJobToFakeDataModel, RenderJob, FakeDataModel)
            else FLAT(RenderJobToRenderView, RenderJob, RenderView)
            else FLAT(RigType, Humanoid, RigType)
            else FLAT(Rotation, Primitive, Rotation)
            else FLAT(ScriptContext, DataModel, ScriptContext)
            else FLAT(Sit, Humanoid, Sit)
            else FLAT(SoundId, Sound, SoundId)
            else FLAT(StringLength, Misc, StringLength)
            else FLAT(TaskSchedulerMaxFPS, TaskScheduler, MaxFPS)
            else FLAT(TaskSchedulerPointer, TaskScheduler, Pointer)
            else FLAT(Team, Player, Team)
            else FLAT(TeamColor, Player, TeamColor)
            else FLAT(TextLabelText, GuiObject, Text)
            else FLAT(TextLabelVisible, GuiObject, Visible)
            else FLAT(Tool_Grip_Position, Tool, Grip)
            else FLAT(Transparency, BasePart, Transparency)
            else FLAT(UserId, Player, UserId)
            else FLAT(Value, Misc, Value)
            else FLAT(Velocity, Primitive, AssemblyLinearVelocity)
            else FLAT(ViewportSize, Camera, ViewportSize)
            else FLAT(VisualEngine, VisualEngine, Pointer)
            else FLAT(VisualEnginePointer, VisualEngine, Pointer)
            else FLAT(VisualEngineToDataModel1, VisualEngine, FakeDataModel)
            else FLAT(VisualEngineToDataModel2, VisualEngine, RenderView)
            else FLAT(WalkSpeed, Humanoid, Walkspeed)
            else FLAT(WalkSpeedCheck, Humanoid, WalkspeedCheck)
            else FLAT(Workspace, DataModel, Workspace)
            else FLAT(viewmatrix, VisualEngine, ViewMatrix)
        }

#undef FLAT
    }

    static std::string ScanRobloxVersion() {
        uint64_t base = Driver->Get_Module();
        HANDLE hProcess = Driver->Get_Handle();
        if (!base || !hProcess) return {};

        const char pattern[] = "version-";
        size_t patLen = sizeof(pattern) - 1;
        char buffer[0x1000];
        ULONG bytesRead = 0;

        for (uint64_t addr = base; addr < base + 0x10000000; addr += sizeof(buffer) - patLen) {
            bytesRead = 0;
            Driver_ReadVirtualMemory(hProcess, (PVOID)addr, buffer, sizeof(buffer), &bytesRead);
            if (bytesRead < patLen) continue;

            for (size_t i = 0; i < bytesRead - patLen; i++) {
                if (memcmp(buffer + i, pattern, patLen) == 0) {
                    size_t end = i + patLen;
                    while (end < bytesRead && buffer[end] >= ' ' && buffer[end] <= '~')
                        end++;
                    return std::string(buffer + i, end - i);
                }
            }
        }
        return {};
    }

    bool Fetch() {
        std::string robloxVer = ScanRobloxVersion();
        if (robloxVer.empty()) {
            Output::Warning("Could not detect Roblox version from process. Using hardcoded defaults.");
            return false;
        }

        Output::Info("Detected version: '{}' (length: {})", robloxVer, robloxVer.size());

        if (robloxVer == Offsets::ClientVersion) {
            Output::Success("Offsets already up to date ({}).", robloxVer);
            return true;
        }

        RobloxVersion = robloxVer;

        std::wstring wVer(robloxVer.begin(), robloxVer.end());
        std::wstring path = L"/" + wVer + L"/offsets.json";
        std::string response = FetchURL(L"offsets.imtheo.lol", path.c_str(), true);

        if (response.empty() || response.find("\"error\"") != std::string::npos) {
            Output::Warning("Offsets for version {} not found.", robloxVer);

            std::string liveVer = FetchURL(L"offsets.imtheo.lol", L"/roblox/version", true);
            while (!liveVer.empty() && (liveVer.back() == '\r' || liveVer.back() == '\n' || liveVer.back() == ' '))
                liveVer.pop_back();

            if (!liveVer.empty() && liveVer != robloxVer) {
                Output::Info("Falling back to live version: {}", liveVer);
                std::wstring wLive(liveVer.begin(), liveVer.end());
                std::wstring livePath = L"/" + wLive + L"/offsets.json";
                response = FetchURL(L"offsets.imtheo.lol", livePath.c_str(), true);
                if (!response.empty() && response.find("\"error\"") == std::string::npos) {
                    RobloxVersion = liveVer;
                }
            }

            if (response.empty() || response.find("\"error\"") != std::string::npos) {
                Output::Warning("Failed to download offsets. Using hardcoded defaults.");
                return false;
            }
        }

        size_t pos = 0;
        JsonValue root = ParseValue(response, pos);
        if (!root.isObject) {
            Output::Warning("Failed to parse offset JSON. Using hardcoded defaults.");
            return false;
        }

        ApplyNested(root);

        Offsets::ClientVersion = RobloxVersion;

        Output::Success("Offsets updated to version {}.", RobloxVersion);
        return true;
    }

}
