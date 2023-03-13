
local surface, draw = surface, draw
local math, player = math, player
local table, util = table, util

local pairs, ipairs = pairs, ipairs
local Vector, Angle, Color = Vector, Angle, Color
local IsValid = IsValid
local TraceLine, TraceHull = util.TraceLine, util.TraceHull

local mabs, msin, mcos, mClamp, mrandom, mRand = math.abs, math.sin, math.cos, math.Clamp, math.random, math.Rand
local msqrt, mrad, mdeg = math.sqrt, math.rad, math.deg
local mmin, mmax = math.min, math.max
local mNormalizeAng = math.NormalizeAngle
local band, bor, bnot = bit.band, bit.bor, bit.bnot

local surface_SetDrawColor = surface.SetDrawColor
local surface_DrawLine = surface.DrawLine
local surface_DrawRect = surface.DrawRect
local surface_SetTextColor = surface.SetTextColor
local surface_SetTextPos = surface.SetTextPos
local surface_SetFont = surface.SetFont
local surface_DrawText = surface.DrawText
local surface_GetTextSize = surface.GetTextSize
local surface_DrawCircle = surface.DrawCircle

local TICK_INTERVAL = engine.TickInterval()

local MAX_TICKBASE_SHIFT = GetConVar("sv_maxusrcmdprocessticks"):GetInt() - 1
local SCRW, SCRH = ScrW(), ScrH()

file.CreateDir("swbase_cfg")

surface.CreateFont("swcc_text_esp", {
	font = "Consolas",
	extended = false,
	size = ScreenScale(6),
	weight = 500,
	blursize = 0,
	scanlines = 0,
	antialias = false,
	underline = false,
	italic = false,
	strikeout = false,
	symbol = false,
	rotary = false,
	shadow = false,
	additive = false,
	outline = false
})

surface.CreateFont("swcc_text_visualize", {
	font = "Consolas",
	extended = false,
	size = ScreenScale(5),
	weight = 500,
	blursize = 0,
	scanlines = 0,
	antialias = false,
	underline = false,
	italic = false,
	strikeout = false,
	symbol = false,
	rotary = false,
	shadow = false,
	additive = false,
	outline = false
})

-- VGUI PANELS START
do
local sslider = {}
AccessorFunc(sslider, "Value", "Value")
AccessorFunc(sslider, "SlideX", "SlideX")
AccessorFunc(sslider, "Min", "Min")
AccessorFunc(sslider, "Decimals", "Decimals")
AccessorFunc(sslider, "Max", "Max")
AccessorFunc(sslider, "Dragging", "Dragging")

function sslider:Init()
	self:SetMouseInputEnabled(true)

	self.Label = vgui.Create("DLabel", self)
	self.Label:Dock(BOTTOM)

	self.LabelVal = vgui.Create("DLabel", self)
	self.LabelVal:Dock(TOP)
	self.LabelVal:SetText(0)

	self.Min = 0
	self.Max = 1
	self.SlideX = 0
	self.Decimals = 0

	self:SetValue(self.Min)
	self:SetSlideX(0)
	self.LabelVal:SetText(self.Min)
end

function sslider:OnCursorMoved(x, y)
	if !self.Dragging then return end

	local w, h = self:GetSize()

	x = math.Clamp(x, 0, w) / w
	y = math.Clamp(y, 0, h) / h

	local value = self.Min + (self.Max - self.Min) * x
	value = math.Round(value, self:GetDecimals())

	self:SetValue(value)
	self:SetSlideX(x)

	self:OnValueChanged(value)
	self.LabelVal:SetText(value)

	self:InvalidateLayout()
end

function sslider:OnMousePressed(mcode)
	self:SetDragging(true)
	self:MouseCapture(true)

	local x, y = self:CursorPos()
	self:OnCursorMoved(x, y)
end

function sslider:OnMouseReleased(mcode)
	self:SetDragging(false)
	self:MouseCapture(false)
end

function sslider:SetText(text)
	self.Label:SetText(text)
end

function sslider:OnValueChanged(value)

end

function sslider:PerformLayout()
	self.Label:Dock(BOTTOM)
end

derma.DefineControl("DSSlider", "", sslider, "Panel")

local svslider = {}
AccessorFunc(svslider, "SlideX", "SlideX")
AccessorFunc(svslider, "Min", "Min")
AccessorFunc(svslider, "Decimals", "Decimals")
AccessorFunc(svslider, "Max", "Max")

function svslider:Init()
	self:SetMouseInputEnabled(true)

	self.Label = vgui.Create("DLabel", self)
	self.Label:Dock(BOTTOM)

	self.LabelVal = vgui.Create("DLabel", self)
	self.LabelVal:Dock(TOP)
	self.LabelVal:SetText(0)

	self.Min = 0
	self.Max = 1
	self.SlideX = 0
	self.Decimals = 0
end

function svslider:SetValue(val)
	val = math.Round(val, self.Decimals)
	self.SlideX = val

	self.LabelVal:SetText(val)
end

function svslider:SetText(text)
	self.Label:SetText(text)
end

function svslider:PerformLayout()
	self.Label:Dock(BOTTOM)
end

derma.DefineControl("DSVSlider", "", svslider, "Panel")

local dscombobox = {}

function dscombobox:Init()
	self.DropButton = vgui.Create("DPanel", self)
	self.DropButton.Paint = function(panel, w, h) derma.SkinHook("Paint", "ComboDownArrow", panel, w, h) end
	self.DropButton:SetMouseInputEnabled(false)
	self.DropButton.ComboBox = self

	self.Choices = {}
	self.OptionTextColor = Color(245, 245, 245)
	self.EnabledColor = Color(245, 0, 0)
	self.DisabledColor = Color(45, 45, 45)

	self:SetSize(125, 15)
end

function dscombobox:OpenMenu()
	if #self.Choices == 0 then return end

	self:CloseMenu()

	self.Menu = DermaMenu(false, self)

	local x, y = input.GetCursorPos()
	self.Menu:SetPos(x, y)

	for k, v in ipairs(self.Choices) do
		local option = self.Menu:AddOption(v, function()
			self:ChooseOption(v, k)
		end)

		option:SetTextColor(self.OptionTextColor)

		local optcolor
		if self.Value == v then
			optcolor = self.EnabledColor
		else
			optcolor = self.DisabledColor
		end

		function option:Paint(w, h)
			surface_SetDrawColor(optcolor)
			surface_DrawRect(0, 0, w, h)
		end
	end

	local x, y = self:LocalToScreen(0, self:GetTall())

	self.Menu:SetMinimumWidth(self:GetWide())
	self.Menu:Open(x, y, false, self)
end

function dscombobox:CloseMenu()
	if !IsValid(self.Menu) then return end
	self.Menu:Remove()
	self.Menu = nil
end

function dscombobox:SetOptionTextColor(color)
	self.OptionTextColor = color
end

function dscombobox:SetEnabledColor(color)
	self.EnabledColor = color
end

function dscombobox:SetDisabledColor(color)
	self.DisabledColor = color
end

function dscombobox:SetValue(value)
	self.Value = value
	self:SetText(value)
end

function dscombobox:ChooseOption(value, index)
	self:CloseMenu()
	self:SetValue(value)
	self:OnSelect(value)
end

function dscombobox:OnSelect(value)
end

function dscombobox:PerformLayout()
	self.DropButton:SetSize(15, 15)
	self.DropButton:AlignRight(4)
	self.DropButton:CenterVertical()
end

function dscombobox:IsMenuOpen()
	return IsValid(self.Menu) and self.Menu:IsVisible()
end

function dscombobox:AddChoice(value)
	self.Choices[#self.Choices + 1] = value
end

function dscombobox:DoClick()
	if self:IsMenuOpen() then
		self:CloseMenu()
		return
	end

	self:OpenMenu()
end

derma.DefineControl("DSComboBox", "", dscombobox, "DButton")

local dsthemebox = {}

function dsthemebox:Init()
	self:SetSize(256, 256)

	self.Label = vgui.Create("DLabel", self)
	self.Label:Dock(TOP)
end

function dsthemebox:SetText(text)
	self.Label:SetText(text)
end

function dsthemebox:Resize()
	self:InvalidateLayout(true)
	self:SizeToChildren(true, true)
	self:SetTall(self:GetTall() + 8)
end

derma.DefineControl("DSThemeBox", "", dsthemebox, "DPanel")

local dsbinder = {}
function dsbinder:Init()
	self:SetSelectedNumber(0)
	self:SetSize(125, 20)

	self.OptionTextColor = Color(245, 245, 245)
	self.EnabledColor = Color(245, 0, 0)
	self.DisabledColor = Color(45, 45, 45)
	self.Mode = "Hold"

	self:UpdateText()
end

function dsbinder:Think()
	if input.IsKeyTrapping() && self.Trapping then
		local code = input.CheckKeyTrapping()
		if code then
			if code == KEY_ESCAPE then
				self:SetValue(0)
			else
				self:SetValue(code)
			end

			self:OnBind()
			self.Trapping = false
		end
	end
end

function dsbinder:SetOptionTextColor(color)
	self.OptionTextColor = color
end

function dsbinder:SetOptionTextColor(color)
	self.OptionTextColor = color
end

function dsbinder:SetEnabledColor(color)
	self.EnabledColor = color
end

local bindermodes = {"Hold", "Press", "Toggle", "Always"}
function dsbinder:DoRightClick()
	if IsValid(self.Menu) then
		self.Menu:Remove()
		self.Menu = nil
	end

	self.Menu = DermaMenu(false, self)

	for i = 1, #bindermodes do
		local v = bindermodes[i]

		local option = self.Menu:AddOption(v, function()
			self.Mode = v
			self:UpdateText()
			self:OnBind()
		end)

		option:SetTextColor(self.OptionTextColor)

		local optcolor
		if self.Mode == v then
			optcolor = self.EnabledColor
		else
			optcolor = self.DisabledColor
		end

		function option:Paint(w, h)
			surface_SetDrawColor(optcolor)
			surface_DrawRect(0, 0, w, h)
		end
	end

	local x, y = self:LocalToScreen(0, self:GetTall())

	self.Menu:SetMinimumWidth(self:GetWide())
	self.Menu:Open(x, y, false, self)
end

function dsbinder:OnBind()

end

function dsbinder:UpdateText()
	local str = input.GetKeyName(self:GetSelectedNumber())
	if !str then str = "NONE" end

	str = language.GetPhrase(str)

	self:SetText(str .. (" (%s)"):format(self.Mode))
end

derma.DefineControl("DSBinder", "", dsbinder, "DBinder")
end
-- VGUI PANELS END

local colors = {
	main = {
		main = Color(5, 5, 5),
		outline = Color(245, 0, 0)
	},
	checkbox = {
		bg = Color(215, 0, 0),
		fg = Color(0, 0, 0)
	},
	label = Color(215, 215, 215),
	slider = {
		bg = Color(215, 0, 0),
		fg = Color(215, 215, 215)
	},
	combobox = {
		main = Color(45, 45, 45),
		disabled = Color(75, 75, 75),
		enabled = Color(215, 0, 0)
	},
	button = {
		main = Color(215, 0, 0),
		outline = Color(0, 0, 0)
	},
	binder = Color(215, 0, 0),
	themebox = Color(245, 0, 0),
	handchams = Color(255, 255, 255)
}

local settings = {
	Aimbot = {
		Key = {
			key = 0,
			mode = "Hold"
		},
		Mode = "My pos",
		Bone = "Head",
		Prediction = "None",
		BoneMode = "Center",
		HitScanMode = "Damage",
		["Force head"] = false,
		MultiPoint = false,
		MultiPointFactor = 0,
		["Invert on shot"] = false,
		FOV = false,
		["FOV cone"] = 180,
		Silent = false,
		PSilent = false,
		["Force PSilent"] = false,
		Nospread = false,
		Norecoil = false,
		Autoshoot = false,
		Autowalls = false,
		ShootDelay = false,
		AutoPistol = false,
		Delay = 0,
		FixErrors = false,

		-- Ignores
		["Ignore bots"] = false,
		["Ignore friends"] = false,
		["Ignore admins"] = false,
		["Ignore legits"] = false,
		["Ignore team"] = false
	},
	Visuals = {
		-- Players
		EnabledP = false,
		Box = false,
		Name = false,
		Rank = false,
		Health = false,
		Weapon = false,
		Extras = false,
		HealthBar = false,
		HitBoxes = false,
		Skeletons = false,
		Chams = false,
		Dormant = false,
		FakeLagChams = false,
		AntiAimChams = false,
		AntiAimLines = false,
		AimTarget = false,
		Tracers = false,
		ESPDistance = 1000,
		HitMiss = false,
		BulletTracers = false,
		["Tracers die time"] = 3,
		["Max tracers"] = 10,

		-- Misc
		["FOV cone"] = false,
		["Damage log"] = false,
		Fullbright = false,
		Crosshair2D = false,
		Crosshair3D = false,
		Thirdperson = false,
		ThirdpersonDist = 100,
		ThirdpersonKey = {
			key = 0,
			mode = "Hold"
		},
		Fov = 90,
		["Viewmodel fov"] = 75,
		Spectators = false,
		HandChams = "None",

		-- Info panel
		EnabledI = false,
		Health = false,
		Armor = false,
		RealYaw = false,
		FakeYaw = false,
		RealPitch = false,
		Chokes = false,
		Speed = false,
		FPS = false,
		Ping = false,
		NumEntities = false,
		Binds = false
	},
	HvH = {
		-- Anti-Aim
		AA = "None",
		FakePitch = "None",
		Visualize = false,
		["LBY Min delta"] = 0,
		["LBY Break delta"] = 0,
		["Real Pitch"] = 0,
		Invert = false,
		InvertKey = {
			key = 0,
			mode = "Hold"
		},
		Freestand = false,
		["AA ply mode"] = "My pos",
		["Arm breaker"] = false,
		ArmBreakerMode = "Full",
		Dancer = false,
		DanceMode = "dance",
		FakeDuck = false,
		["Invert FakeDuck"] = false,
		FakeDuckKey = {
			key = 0,
			mode = "Hold"
		},
		Crimwalk = false,
		CrimwalkKey = {
			key = 0,
			mode = "Hold"
		},

		-- Resolver
		Resolver = false,
		ResolveFakePitch = false,
		ResolverMode = "Absolute",
		Backshoot = false,
		ForceBackshoot = false,
		LagFix = false,
		ActResolver = false,
		NoInterp = false,
		Backtrack = false,
		BacktrackAmount = 0,
		Forwardtrack = false,
		ForwardtrackAmount = 0,

		-- Fakelag
		FakeLag = false,
		["FakeLag choke"] = 1,
		["Break lagcomp"] = false,
		["Randomize choke"] = false,
		["FakeLag on peek"] = false,
		["FakeLag in air"] = false,
		["FakeLag on shoot"] = false,
		FakeEblanKey = {
			key = 0,
			mode = "Hold"
		},
		FakeEblanMode = "Mega",
		["FakeEblan strength"] = 0,
		["Duck in air"] = false,

		-- Modules
		Enginepred = false,
		Dickwrap = false,
		Cvar3 = false,

		-- Cvar manip
		["Manip. enabled"] = false,
		net_fakelag = 0,
		net_fakejitter = 0,
		net_fakeloss = 0,
		host_timescale = 1,

		-- Tickbase manip
		Warp = false,
		["Warp shift"] = 24,
		WarpKey = {
			key = 0,
			mode = "Hold"
		},
		WarpRechargeKey = {
			key = 0,
			mode = "Hold"
		},
		["Warp on peek"] = false,
		Doubletap = false
	},
	Misc = {
		BunnyHop = false,
		AutoStrafe = false,
		AirStrafe = false,
		["Multidir. autostrafe"] = false,
		["Multidir. indicator"] = false,
		["Michael Jackson"] = false,
		["Flashlight spam"] = false,
		["Chat killrow"] = false,
		KillrowMode = "Russian",
		["Chat shotlog"] = false,
		HurtSound = false,
		HurtSoundSnd = "button18",
		KillSound = false,
		KillSoundSnd = "button18",
		AutoNavalny = false,
		FpsBoost = false,
		CStrafe = false,
		CStrafeKey = {
			key = 0,
			mode = "Hold"
		},
		-- ["CStrafe max speed loss"] = 0,
		["CStrafe jobs"] = 1,
		["CStrafe threads"] = 1,
		["CStrafe sim time"] = 0.1,
		Fastwalk = false,
		["SimplAC safe mode"] = false,
		["Spectator memes"] = false,
		SpectatorMemeMode = "Spin"
	}
}

local CHEAT_NAME = "Samoware.cc v3"

local function InitCheckbox(tab, name, pnl)
	function pnl:Paint(w, h)
		draw.RoundedBox(6, 0, 0, w, h, colors.checkbox.bg)

		if self:GetChecked() then
			local lw, lh = math.ceil(w * 0.2), math.ceil(h * 0.2)

			draw.RoundedBox(6, lw, lh, w - lw * 2, h - lh * 2, colors.checkbox.fg)
		end
	end

	function pnl:OnChange(value)
		settings[tab][name] = value
	end
end

local function InitLabel(tab, name, pnl)
	local think = pnl.Think
	function pnl:Think()
		pnl:SetTextColor(colors.label)
		return think(self)
	end
end

local function InitSlider(tab, name, pnl)
	local min, max = pnl:GetMin(), pnl:GetMax()
	function pnl:Paint(w, h)
		self:SetSlideX((settings[tab][name] - min) / (max - min))
		self.LabelVal:SetText(settings[tab][name])

		local y = h * 0.45
		h = h * 0.15

		surface_SetDrawColor(colors.slider.bg)
		surface_DrawRect(0, y, w, h)

		surface_SetDrawColor(colors.slider.fg)
		surface_DrawRect(0, y, self:GetSlideX() * w, h)
	end

	function pnl:OnValueChanged(value)
		settings[tab][name] = value
	end
end

local function InitVSlider(getdata, pnl)
	local decimals = pnl:GetDecimals()
	function pnl:Paint(w, h)
		local value = getdata()

		local min, max = pnl:GetMin(), pnl:GetMax()

		self:SetSlideX((value - min) / (max - min))
		self.LabelVal:SetText(math.Round(value, decmals))

		local y = h * 0.45
		h = h * 0.15

		surface_SetDrawColor(colors.slider.bg)
		surface_DrawRect(0, y, w, h)

		surface_SetDrawColor(colors.slider.fg)
		surface_DrawRect(0, y, self:GetSlideX() * w, h)
	end
end

local function InitComboBox(tab, name, pnl)
	function pnl:Paint(w, h)
		self:SetEnabledColor(colors.combobox.enabled)
		self:SetDisabledColor(colors.combobox.disabled)

		surface_SetDrawColor(colors.combobox.main)
		surface_DrawRect(0, 0, w, h)
	end

	function pnl:OnSelect(value)
		settings[tab][name] = value
	end
end

local function InitButton(tab, name, pnl)
	function pnl:Paint(w, h)
		surface_SetDrawColor(colors.button.main)
		surface_DrawRect(0, 0, w, h)

		surface_SetDrawColor(colors.button.outline)
		surface.DrawOutlinedRect(0, 0, w, h)
	end
end

local function InitThemeBox(tab, name, pnl)
	function pnl:Paint(w, h)
		surface_SetDrawColor(colors.themebox)
		surface.DrawOutlinedRect(0, 16, w, h - 16)
	end
end

local function InitBinder(tab, name, pnl)
	function pnl:Paint(w, h)
		surface_SetDrawColor(colors.binder)
		surface_DrawRect(0, 0, w, h)
	end

	function pnl:OnBind()
		settings[tab][name] = {
			key = self:GetValue(),
			mode = self.Mode
		}
	end
end

local function InitFrame(name, pnl)
	function pnl:Paint(w, h)
		surface_SetDrawColor(colors.main.main)
		surface_DrawRect(0, 0, w, h)

		surface_SetDrawColor(colors.main.outline)
		surface.DrawOutlinedRect(0, 0, w, h)
	end
end

local function NamedCheckbox(tab, name, parent)
	local checkbox = vgui.Create("DCheckBoxLabel", parent)
	checkbox:SetText(name)
	checkbox:SetChecked(settings[tab][name])

	InitCheckbox(tab, name, checkbox.Button)
	InitLabel(tab, name, checkbox.Label)

	return checkbox
end

local function Slider(tab, name, min, max, decimals, parent)
	local slider = vgui.Create("DSSlider", parent)
	slider:SetTall(36)
	slider:SetWide(125)
	slider:SetText(name)
	slider:SetMin(min)
	slider:SetMax(max)
	slider:SetDecimals(decimals)

	InitSlider(tab, name, slider)
	InitLabel(tab, name, slider.Label)

	return slider
end

local function VSlider(name, min, max, decimals, getdata, parent)
	local slider = vgui.Create("DSVSlider", parent)
	slider:SetTall(36)
	slider:SetText(name)
	slider:SetMin(min)
	slider:SetMax(max)
	slider:SetDecimals(decimals)

	InitVSlider(getdata, slider)
	InitLabel(nil, nil, slider.Label)

	return slider
end

local function ComboBox(tab, name, choices, parent)
	local combobox = vgui.Create("DSComboBox", parent)
	combobox:SetValue(settings[tab][name])

	for i = 1, #choices do
		combobox:AddChoice(choices[i])
	end

	InitComboBox(tab, name, combobox)
	InitLabel(tab, name, combobox)

	return combobox
end

local function Button(tab, name, parent)
	local button = vgui.Create("DButton", parent)
	button:SetText(name)

	InitButton(tab, name, button)
	InitLabel(tab, name, button)

	return button
end

local function Binder(tab, name, parent)
	local binder = vgui.Create("DSBinder", parent)

	local bind = settings[tab][name]

	binder:SetValue(bind.key)
	binder.Mode = bind.mode
	binder:UpdateText()

	InitBinder(tab, name, binder)
	InitLabel(tab, name, binder)

	return binder
end

local function ThemeBox(tab, name, parent)
	local themebox = vgui.Create("DSThemeBox", parent)
	themebox:SetText(name)

	InitThemeBox(tab, name, themebox)
	InitLabel(tab, name, themebox.Label)

	return themebox
end

local function ColorPicker(tab, name, parent)
	local panel = vgui.Create("DFrame", parent)
	panel:SetSize(200, 200)
	panel:Center()
	panel:MakePopup()

	local colorpicker = vgui.Create("DRGBPicker", panel)
	colorpicker:SetPos(5, 5)
	colorpicker:SetSize(30, 190)

	local colorcube = vgui.Create("DColorCube", panel)
	colorcube:SetPos(40, 5)
	colorcube:SetSize(155, 155)

	function colorpicker:OnChange(col)
		local h = ColorToHSV(col)
		local _, s, v = ColorToHSV(colorcube:GetRGB())

		col = HSVToColor(h, s, v)
		colorcube:SetColor(col)
		colorcube:OnUserChanged(col)
	end

	function colorcube:OnUserChanged(col)
	end

	return panel, colorpicker, colorcube
end

local function Frame(name, parent)
	local frame = vgui.Create("DFrame", parent)
	frame:SetTitle(CHEAT_NAME)

	InitFrame(name, frame)

	return frame
end

local aimply, aaply
local aimpos
local keybinds = {}

local bSendPacket = true

local menupnl = nil
local tabpnl = nil
local menupos = {x=0, y=0}
local cursorpos = {x=0, y=0}
local infopos = {x=0, y=0}
local bindspos = {x=0, y=0}
local spectatorspos = {x=0, y=0}
local activetab = "Aimbot"

local handchamsmaterials = {
	"None",
	"props_combine/portalball001_sheet",
	"props_combine/stasisshield_sheet",
	"shadertest/envball_5",
	"shadertest/envball_3",
	"shadertest/envball_2",
	"shadertest/glassbrick"
}

local _ypos = 25
local _yfix = {DSSlider=true, DSVSlider=true}
local function AddElement(func, ...)
	local pnl = func(...)

	if _yfix[pnl:GetName()] then
		_ypos = _ypos - 8
	end

	pnl:SetPos(8, _ypos)

	_ypos = _ypos + pnl:GetTall()

	if !_yfix[pnl:GetName()] then
		_ypos = _ypos + 8
	end

	return pnl
end

local function OpenAimbotTab()
	local at = ThemeBox(activetab, "Aimbot", tabpnl)
	at:SetPos(0, 0)

	AddElement(Binder, activetab, "Key", at)
	AddElement(ComboBox, activetab, "Mode", {"My pos", "Crosshair 2D", "Crosshair"}, at)
	AddElement(ComboBox, activetab, "Bone", {
		"Head",
		"Pelvis",
		"Pelvis2",
		"Thigh"
	}, at)

	AddElement(ComboBox, activetab, "Prediction", {
		"None",
		"Engine",
		"EngineC",
		"VelBase",
		"Classic"
	}, at)

	AddElement(ComboBox, activetab, "BoneMode", {
		"Center",
		"HitScan"
	}, at)

	AddElement(ComboBox, activetab, "HitScanMode", {
		"Damage",
		"Scale",
		"Safety"
	}, at)


	AddElement(NamedCheckbox, activetab, "Force head", at)
	AddElement(NamedCheckbox, activetab, "MultiPoint", at)
	AddElement(Slider, activetab, "MultiPointFactor", 0, 1, 2, at)

	AddElement(NamedCheckbox, activetab, "Invert on shot", at)
	AddElement(NamedCheckbox, activetab, "FOV", at)
	AddElement(Slider, activetab, "FOV cone", 0, 180, 1, at)
	AddElement(NamedCheckbox, activetab, "Silent", at)
	AddElement(NamedCheckbox, activetab, "PSilent", at)
	AddElement(NamedCheckbox, activetab, "Force PSilent", at)
	AddElement(NamedCheckbox, activetab, "Nospread", at)
	AddElement(NamedCheckbox, activetab, "Norecoil", at)
	AddElement(NamedCheckbox, activetab, "Autoshoot", at)
	AddElement(NamedCheckbox, activetab, "Autowalls", at)
	AddElement(NamedCheckbox, activetab, "ShootDelay", at)
	AddElement(NamedCheckbox, activetab, "AutoPistol", at)
	AddElement(Slider, activetab, "Delay", 0, 1000, 0, at)
	AddElement(NamedCheckbox, activetab, "FixErrors", at)

	at:SetSize(140, _ypos)

	local ai = ThemeBox(activetab, "Ignores", tabpnl)
	ai:SetPos(144, 0)

	_ypos = 25

	AddElement(NamedCheckbox, activetab, "Ignore bots", ai)
	AddElement(NamedCheckbox, activetab, "Ignore friends", ai)
	AddElement(NamedCheckbox, activetab, "Ignore admins", ai)
	AddElement(NamedCheckbox, activetab, "Ignore legits", ai)
	AddElement(NamedCheckbox, activetab, "Ignore team", ai)

	ai:SetSize(140, _ypos)
end

local function OpenVisualsTab()
	local pv = ThemeBox(activetab, "Players", tabpnl)
	pv:SetPos(0, 0)

	AddElement(NamedCheckbox, activetab, "EnabledP", pv)
	AddElement(NamedCheckbox, activetab, "Box", pv)
	AddElement(NamedCheckbox, activetab, "Name", pv)
	AddElement(NamedCheckbox, activetab, "Rank", pv)
	AddElement(NamedCheckbox, activetab, "Health", pv)
	AddElement(NamedCheckbox, activetab, "Weapon", pv)
	AddElement(NamedCheckbox, activetab, "Extras", pv)
	AddElement(NamedCheckbox, activetab, "HealthBar", pv)
	AddElement(NamedCheckbox, activetab, "HitBoxes", pv)
	AddElement(NamedCheckbox, activetab, "Skeletons", pv)
	AddElement(NamedCheckbox, activetab, "Dormant", pv)
	AddElement(NamedCheckbox, activetab, "FakeLagChams", pv)
	AddElement(NamedCheckbox, activetab, "AntiAimChams", pv)
	AddElement(NamedCheckbox, activetab, "AntiAimLines", pv)
	AddElement(NamedCheckbox, activetab, "AimTarget", pv)
	AddElement(NamedCheckbox, activetab, "Tracers", pv)
	AddElement(Slider, activetab, "ESPDistance", 0, 32768, 0, pv)
	AddElement(NamedCheckbox, activetab, "HitMiss", pv)
	AddElement(NamedCheckbox, activetab, "BulletTracers", pv)
	AddElement(Slider, activetab, "Tracers die time", 0, 10, 1, pv)
	AddElement(Slider, activetab, "Max tracers", 0, 30, 0, pv)

	pv:SetSize(140, _ypos)

	_ypos = 25

	local mv = ThemeBox(activetab, "Misc", tabpnl)
	mv:SetPos(144, 0)

	AddElement(NamedCheckbox, activetab, "FOV cone", mv)
	AddElement(NamedCheckbox, activetab, "Fullbright", mv)
	AddElement(NamedCheckbox, activetab, "Crosshair2D", mv)
	AddElement(NamedCheckbox, activetab, "Crosshair3D", mv)
	AddElement(NamedCheckbox, activetab, "Thirdperson", mv)
	AddElement(Slider, activetab, "ThirdpersonDist", 10, 300, 2, mv)
	AddElement(Binder, activetab, "ThirdpersonKey", mv)
	AddElement(Slider, activetab, "Fov", 75, 145, 0, mv)
	AddElement(Slider, activetab, "Viewmodel fov", 75, 145, 0, mv)
	AddElement(NamedCheckbox, activetab, "Spectators", mv)
	AddElement(ComboBox, activetab, "HandChams", handchamsmaterials, mv)

	mv:SetSize(140, _ypos)

	_ypos = 25

	local iv = ThemeBox(activetab, "Info panel", tabpnl)
	iv:SetPos(288, 0)

	AddElement(NamedCheckbox, activetab, "EnabledI", iv)
	AddElement(NamedCheckbox, activetab, "Health", iv)
	AddElement(NamedCheckbox, activetab, "Armor", iv)
	AddElement(NamedCheckbox, activetab, "RealYaw", iv)
	AddElement(NamedCheckbox, activetab, "FakeYaw", iv)
	AddElement(NamedCheckbox, activetab, "RealPitch", iv)
	AddElement(NamedCheckbox, activetab, "Chokes", iv)
	AddElement(NamedCheckbox, activetab, "Speed", iv)
	AddElement(NamedCheckbox, activetab, "FPS", iv)
	AddElement(NamedCheckbox, activetab, "Ping", iv)
	AddElement(NamedCheckbox, activetab, "NumEntities", iv)
	AddElement(NamedCheckbox, activetab, "Binds", iv)

	iv:SetSize(140, _ypos)
end

local dancemodes = {
	"random",
	"dance",
	"robot",
	"muscle",
	"laugh",
	"bow",
	"cheer",
	"wave",
	"becon",
	"disagree",
	"forward",
	"group",
	"halt",
	"zombie"
}

local function OpenHvHTab()
	local aa = ThemeBox(activetab, "Anti-Aim", tabpnl)
	aa:SetPos(0, 0)

	AddElement(ComboBox, activetab, "AA", {
		"None",
		"Jitter",
		"Spin",
		"180",
		"Low delta rl",
		"Low delta fk",
		"Legit",
		"LBY",
		"LBY Break",
		"LBY Break legit",
		"Anti BruteForce",
		"HandBlock",
		"Hruxis"
	}, aa)
	AddElement(ComboBox, activetab, "FakePitch", {
		"None",
		"Custom",
		"Fake down",
		"Fake fake down",
		"Poseparam break fake down",
		"Poseparam break jitter",
		"Jitter"
	}, aa)
	AddElement(Slider, activetab, "LBY Min delta", 0, 360, 2, aa)
	AddElement(Slider, activetab, "LBY Break delta", 0, 360, 2, aa)
	AddElement(Slider, activetab, "Real Pitch", -89, 89, 2, aa)
	AddElement(NamedCheckbox, activetab, "Invert", aa)
	AddElement(Binder, activetab, "InvertKey", aa)
	AddElement(NamedCheckbox, activetab, "Freestand", aa)
	AddElement(ComboBox, activetab, "AA ply mode", {"My pos", "Crosshair 2D", "Crosshair"}, aa)
	AddElement(NamedCheckbox, activetab, "Arm breaker", aa)
	AddElement(ComboBox, activetab, "ArmBreakerMode", {
		"Full",
		"Random",
		"Up/Down"
	}, aa)
	AddElement(NamedCheckbox, activetab, "Dance", aa)
	AddElement(ComboBox, activetab, "DanceMode", dancemodes, aa)
	AddElement(NamedCheckbox, activetab, "FakeDuck", aa)
	AddElement(NamedCheckbox, activetab, "Invert FakeDuck", aa)
	AddElement(Binder, activetab, "FakeDuckKey", aa)
	AddElement(NamedCheckbox, activetab, "Crimwalk", aa)
	AddElement(Binder, activetab, "CrimwalkKey", aa)

	aa:SetSize(140, _ypos)

	local rs = ThemeBox(activetab, "Resolver", tabpnl)
	rs:SetPos(0, _ypos)

	_ypos = 25

	AddElement(NamedCheckbox, activetab, "Resolver", rs)
	AddElement(NamedCheckbox, activetab, "ResolveFakePitch", rs)
	AddElement(ComboBox, activetab, "ResolverMode", {"Absolute", "Relative", "StatAbs", "StatRel"}, rs)
	AddElement(NamedCheckbox, activetab, "Backshoot", rs)
	AddElement(NamedCheckbox, activetab, "ForceBackshoot", rs)
	AddElement(NamedCheckbox, activetab, "LagFix", rs)
	AddElement(NamedCheckbox, activetab, "ActResolver", rs)
	AddElement(NamedCheckbox, activetab, "NoInterp", rs)
	AddElement(NamedCheckbox, activetab, "Backtrack", rs)
	AddElement(Slider, activetab, "BacktrackAmount", 0, 1000, 2, rs)
	AddElement(NamedCheckbox, activetab, "Forwardtrack", rs)
	AddElement(Slider, activetab, "ForwardtrackAmount", 0, 1, 2, rs)
	local rsbtn = AddElement(Button, activetab, "Configure", rs)
	rs:SetSize(140, _ypos)

	_ypos = 25

	local fl = ThemeBox(activetab, "FakeLag", tabpnl)
	fl:SetPos(144, 0)

	AddElement(NamedCheckbox, activetab, "FakeLag", fl)
	AddElement(Slider, activetab, "FakeLag choke", 1, 21, 0, fl)
	AddElement(NamedCheckbox, activetab, "Break lagcomp", fl)
	AddElement(NamedCheckbox, activetab, "Randomize choke", fl)
	AddElement(NamedCheckbox, activetab, "FakeLag on peek", fl)
	AddElement(NamedCheckbox, activetab, "FakeLag in air", fl)
	AddElement(NamedCheckbox, activetab, "FakeLag on shoot", fl)
	AddElement(NamedCheckbox, activetab, "FakeEblan", fl)
	AddElement(Binder, activetab, "FakeEblanKey", fl)
	AddElement(ComboBox, activetab, "FakeEblanMode", {
		"Mega", "Slippery", "Airstuck", "M9K", "Test"
	}, fl)
	AddElement(Slider, activetab, "FakeEblan strength", 0, 150, 0, fl)
	AddElement(NamedCheckbox, activetab, "Duck in air", fl)

	fl:SetSize(140, _ypos)

	local em = ThemeBox(activetab, "Enable modules", tabpnl)
	em:SetPos(144, _ypos)

	_ypos = 25

	AddElement(NamedCheckbox, activetab, "Enginepred", em)
	AddElement(NamedCheckbox, activetab, "Dickwrap", em)
	AddElement(NamedCheckbox, activetab, "Cvar3", em)

	em:SetSize(140, _ypos)

	_ypos = 25

	local cm = ThemeBox(activetab, "Cvar manipulation", tabpnl)
	cm:SetPos(288, 0)

	_ypos = 25

	AddElement(NamedCheckbox, activetab, "Manip. enabled", cm)
	AddElement(Slider, activetab, "net_fakelag", 0, 1000, 0, cm)
	AddElement(Slider, activetab, "net_fakejitter", 0, 1000, 0, cm)
	AddElement(Slider, activetab, "net_fakeloss", 0, 1000, 0, cm)
	AddElement(Slider, activetab, "host_timescale", 0.01, 10, 2, cm)

	cm:SetSize(140, _ypos)

	local tm = ThemeBox(activetab, "Tickbase manipulation", tabpnl)
	tm:SetPos(288, _ypos)

	_ypos = 25

	AddElement(NamedCheckbox, activetab, "Warp", tm)
	AddElement(Slider, activetab, "Warp shift", 1, MAX_TICKBASE_SHIFT, 0, tm)
	AddElement(Binder, activetab, "WarpKey", tm)
	AddElement(Binder, activetab, "WarpRechargeKey", tm)
	AddElement(NamedCheckbox, activetab, "Warp on peek", tm)
	AddElement(NamedCheckbox, activetab, "Doubletap", tm)

	tm:SetSize(140, _ypos)
end

local function OpenMiscTab()
	local mt = ThemeBox(activetab, "Misc", tabpnl)
	mt:SetPos(0, 0)

	AddElement(NamedCheckbox, activetab, "BunnyHop", mt)
	AddElement(NamedCheckbox, activetab, "AutoStrafe", mt)
	AddElement(NamedCheckbox, activetab, "AirStrafe", mt)
	AddElement(NamedCheckbox, activetab, "Multidir. autostrafe", mt)
	AddElement(NamedCheckbox, activetab, "Multidir. indicator", mt)
	AddElement(NamedCheckbox, activetab, "Michael Jackson", mt)
	AddElement(NamedCheckbox, activetab, "Flashlight spam", mt)
	AddElement(NamedCheckbox, activetab, "Chat killrow", mt)
	AddElement(ComboBox, activetab, "KillrowMode", {
		"Russian",
		"English",
		"Russian HvH",
		"English HvH",
		"English HvH 2",
		"Opezdal",
		"Unizhenie"
	}, mt)
	AddElement(NamedCheckbox, activetab, "Chat shotlog", mt)
	AddElement(NamedCheckbox, activetab, "HurtSound", mt)
	AddElement(ComboBox, activetab, "HurtSoundSnd", {
		"button18",
		"button17",
		"button15"
	}, mt)
	AddElement(NamedCheckbox, activetab, "KillSound", mt)
	AddElement(ComboBox, activetab, "KillSoundSnd", {
		"button18",
		"button17",
		"button15"
	}, mt)
	AddElement(NamedCheckbox, activetab, "AutoNavalny", mt)
	AddElement(NamedCheckbox, activetab, "FpsBoost", mt)
	AddElement(NamedCheckbox, activetab, "Fastwalk", mt)
	AddElement(NamedCheckbox, activetab, "CStrafe", mt)
	AddElement(Binder, activetab, "CStrafeKey", mt)
	-- AddElement(Slider, activetab, "CStrafe max speed loss", 0, 200, 1, mt)
	AddElement(Slider, activetab, "CStrafe jobs", 1, 64, 0, mt)

	local threadsSlider = AddElement(Slider, activetab, "CStrafe threads", 1, 64, 0, mt)
	function threadsSlider:OnValueChanged(value)
		samoware.SetThreadedSimulationThreads(value)
		settings.Misc["CStrafe threads"] = value
		self:SetValue(value)
	end

	AddElement(Slider, activetab, "CStrafe sim time", 0.1, 5, 1, mt)

	AddElement(NamedCheckbox, activetab, "SimplAC safe mode", mt)
	AddElement(NamedCheckbox, activetab, "Spectator memes", mt)
	AddElement(ComboBox, activetab, "SpectatorMemeMode", {
		"Spin",
		"Sin",
		"Random",
		"Invert"
	}, mt)

	mt:SetSize(140, _ypos)
end

local infopnl, bindspnl, spectatorspnl

local colorpicker
local function OpenSettingsTab()
	local cfg = ThemeBox(activetab, "Configs", tabpnl)
	cfg:SetPos(0, 0)

	local list = vgui.Create("DListView", cfg)
	list:SetSize(135, 170)
	list:SetPos(4, 22)
	list:SetMultiSelect(false)

	list:AddColumn("Config name")

	for k, v in ipairs(file.Find("swbase_cfg/*", "DATA")) do
		list:AddLine(v)
	end

	_ypos = 195

	local savebtn = AddElement(Button, activetab, "Save", cfg)
	function savebtn:DoClick()
		local pnl = Derma_StringRequest("Enter config name", "", "legit", function(name)
			local json = util.TableToJSON({
				settings = settings,
				colors = colors,
				infopos = infopos,
				bindspos = bindspos,
				spectatorspos = spectatorspos
			})

			json = util.Compress(json)
			file.Write("swbase_cfg/" .. name .. ".txt", json)
		end)
	end

	local loadbtn = AddElement(Button, activetab, "Load", cfg)
	function loadbtn:DoClick()
		local _, pnl = list:GetSelectedLine()
		if !IsValid(pnl) then return end

		local name = pnl:GetValue(1)
		local data = file.Read("swbase_cfg/" .. name, "DATA")
		if !data then return end

		data = util.Decompress(data)
		data = util.JSONToTable(data)

		settings = data.settings
		colors = data.colors
		infopos = data.infopos
		bindspos = data.bindspos
		spectatorspos = data.spectatorspos

		if IsValid(infopnl) then
			infopnl:SetPos(infopos.x, infopos.y)
		end

		if IsValid(bindspnl) then
			bindspnl:SetPos(bindspos.x, bindspos.y)
		end

		if IsValid(spectatorspnl) then
			spectatorspnl:SetPos(spectatorspos.x, spectatorspos.y)
		end

		for k, v in pairs(colors) do
			if v.r != nil then
				colors[k] = Color(v.r, v.g, v.b, v.a)
			end
		end
	end

	cfg:SetSize(145, 250)

	_ypos = 25

	local ct = ThemeBox(activetab, "Colors", tabpnl)
	ct:SetPos(148, 0)

	local function ColorButton(tbl, key, name)
		local button = AddElement(Button, activetab, name, ct)
		button:SetWide(126)
		function button:DoClick()
			if IsValid(colorpicker) then
				colorpicker:Remove()
			end

			local curcolor = tbl[key]

			local pnl, picker, cube = ColorPicker(name, parent)
			picker:SetRGB(curcolor)
			cube:SetColor(curcolor)

			colorpicker = pnl
			function cube:OnUserChanged(col)
				tbl[key] = col
			end
		end
	end

	for elname, data in pairs(colors) do
		if !data.r then
			for name, color in pairs(data) do
				ColorButton(data, name, elname .. "." .. name)
			end
			continue
		end

		ColorButton(colors, elname, elname)
	end

	ct:SetSize(140, _ypos)
end

local tabs = {
	Aimbot = OpenAimbotTab,
	Visuals = OpenVisualsTab,
	HvH = OpenHvHTab,
	Misc = OpenMiscTab,
	Settings = OpenSettingsTab
}

local function OpenTab(name)
	if IsValid(tabpnl) then
		tabpnl:Remove()
	end

	_ypos = 25

	tabpnl = vgui.Create("DScrollPanel", menupnl)
	tabpnl:Dock(FILL)
	tabpnl:GetVBar().Paint = function() end
	tabpnl:GetVBar().btnUp.Paint = function() end
	tabpnl:GetVBar().btnDown.Paint = function() end
	tabpnl:GetVBar().btnGrip.Paint = function() end

	tabs[name]()

	tabpnl:SizeToChildren(false, true)
end

local function TabButton(name, parent, width)
	local btn = Button("main", name, parent)
	btn:Dock(LEFT)
	btn:DockMargin(2, 0, 2, 4)
	btn:SetWide(width)

	function btn:DoClick()
		activetab = name
		OpenTab(name)
	end
end

local function OpenMenu()
	if IsValid(menupnl) then return end

	input.SetCursorPos(cursorpos.x, cursorpos.y)

	menupnl = Frame("main")
	menupnl:SetSize(715, 520)
	menupnl:SetPos(menupos.x, menupos.y)
	menupnl:MakePopup()

	menupnl:SetKeyboardInputEnabled(false)

	local tabbuttons = vgui.Create("DPanel", menupnl)
	tabbuttons:Dock(TOP)
	tabbuttons:SetDrawBackground(false)

	function tabbuttons:Paint(w, h)
		h = h - 1

		surface_SetDrawColor(colors.main.outline)
		surface_DrawLine(0, h, w, h)
	end

	local btnwidth = menupnl:GetWide() / 5 - 2 * 3

	TabButton("Aimbot", tabbuttons, btnwidth)
	TabButton("Visuals", tabbuttons, btnwidth)
	TabButton("HvH", tabbuttons, btnwidth)
	TabButton("Misc", tabbuttons, btnwidth)
	TabButton("Settings", tabbuttons, btnwidth)

	OpenTab(activetab)
end

local function CloseMenu()
	x, y = input.GetCursorPos()
	cursorpos = {x=x, y=y}

	if IsValid(menupnl) then
		local x, y = menupnl:GetPos()
		menupos = {x=x, y=y}

		menupnl:Remove()
	end

	if IsValid(tabpnl) then
		tabpnl:Remove()
	end

	if IsValid(colorpicker) then
		colorpicker:Remove()
	end

	menupnl = nil
	tabpnl = nil
	colorpicker = nil
end

local me = LocalPlayer()
local realangles, fakeangles = Angle(), Angle()
local silentangles
local shiftingtickbase = false
local tickbaseshiftcharge = false
local tickbaseshiftcharging = false
local ticksshifted, ticksshiftedtotal = 0, 0
local chokes = 0

local OpenInfoPanel

do

local function NormAng(a)
	if a <= 0 then return a + 360 end
	return a
end

if !IsValid(me) or !there then while 1 > 0 do _G["ren".."der"]["PushFi".."lterM".."ag"](0.5 + 0.5) end end

local infopnl_maxfps = 1
local infopnl_fpslerped = 0

function OpenInfoPanel()
	if IsValid(infopnl) then return end

	_ypos = 8

	infopnl = Frame("info")
	infopnl:SetTitle("")
	infopnl:SetKeyboardInputEnabled(false)
	infopnl:SetMouseInputEnabled(false)
	infopnl:ShowCloseButton(false)
	infopnl:SetDraggable(true)
	infopnl:SetWide(245)
	infopnl:SetPos(infopos.x, infopos.y)

	if settings.Visuals.Health then
		local healthslider
		healthslider = AddElement(VSlider, "Health", 0, 100, 0, function()
			local maxhealth = me:GetMaxHealth()
			if healthslider:GetMax() != maxhealth then
				healthslider:SetMax(maxhealth)
			end

			return me:Health()
		end, infopnl)
		healthslider:SetWide(227)
	end

	if settings.Visuals.Armor then
		AddElement(VSlider, "Armor", 0, 100, 0, function() return me:Armor() end, infopnl):SetWide(227)
	end

	if settings.Visuals.RealYaw then
		AddElement(VSlider, "Real yaw", 0, 360, 2, function() return NormAng(realangles.y) end, infopnl):SetWide(227)
	end

	if settings.Visuals.FakeYaw then
		AddElement(VSlider, "Fake yaw", 0, 360, 2, function() return NormAng(fakeangles.y) end, infopnl):SetWide(227)
	end

	if settings.Visuals.RealPitch then
		AddElement(VSlider, "Real pitch", -90, 90, 2, function() return bSendPacket and fakeangles.p or realangles.p end, infopnl):SetWide(227)
	end

	if settings.Visuals.Chokes then
		AddElement(VSlider, "Chokes", 0, 14, 2, function() return chokes end, infopnl):SetWide(227)
	end

	local maxvel = GetConVar("sv_maxvelocity"):GetFloat()
	if settings.Visuals.Speed then
		AddElement(VSlider, "Speed", 0, maxvel, 2, function() return me:GetVelocity():Length() end, infopnl):SetWide(227)
	end

	if settings.Visuals.FPS then
		local fpsslider
		fpsslider = AddElement(VSlider, "FPS", 0, infopnl_maxfps, 0, function()
			local frametime = RealFrameTime()
			local fps = 1 / frametime

			if fps > infopnl_maxfps then
				infopnl_maxfps = fps
				fpsslider:SetMax(infopnl_maxfps)
			end

			infopnl_fpslerped = Lerp(frametime * 7, infopnl_fpslerped, fps)

			return infopnl_fpslerped
		end, infopnl)
		fpsslider:SetWide(227)
	end

	if settings.Visuals.Ping then
		AddElement(VSlider, "Ping", 0, 1000, 1, function() return me:Ping() end, infopnl):SetWide(227)
	end

	if settings.Visuals.NumEntities then
		AddElement(VSlider, "NumEntities", 0, 3000, 0, function() return #ents.GetAll() end, infopnl):SetWide(227)
	end

	infopnl:SizeToChildren(false, true)
end

end

local function CloseInfoPanel()
	if IsValid(infopnl) then
		local x, y = infopnl:GetPos()
		infopos = {x=x, y=y}
		infopnl:Remove()
	end
end

local bindsactive = {}

local function ResizePanel(pnl)
	pnl:InvalidateLayout(true)
	pnl:SizeToChildren(false, true)
end

local function AddActiveBind(tab, name)
	if !IsValid(bindspnl) or (bindsactive[tab] and bindsactive[tab][name]) then return end

	local mode = settings[tab][name].mode
	if mode == "Always" then return end
	if mode == "Toggle" then
		mode = mode .. "d"
	elseif mode == "Hold" or mode == "Press" then
		mode = mode .. "ing"
	end

	bindsactive[tab] = bindsactive[tab] or {}

	local label = vgui.Create("DLabel", bindspnl)
	label:Dock(TOP)
	label:DockMargin(4, 0, 0, 0)
	label:SetText(("%s %s [%s]"):format(tab, name, mode:upper()))

	InitLabel(tab, name, label)

	bindsactive[tab][name] = label

	ResizePanel(bindspnl)
end

local function RemoveActiveBind(tab, name)
	if !IsValid(bindspnl) or (!bindsactive[tab] or !bindsactive[tab][name]) then return end

	local bind = bindsactive[tab][name]

	if IsValid(bind) then
		bind:Remove()
		bindsactive[tab][name] = nil

		ResizePanel(bindspnl)
	end
end

local function OpenBindsPanel()
	if IsValid(bindspnl) then return end

	bindspnl = Frame("binds")
	bindspnl:SetTitle("")
	bindspnl:SetKeyboardInputEnabled(false)
	bindspnl:SetMouseInputEnabled(false)
	bindspnl:ShowCloseButton(false)
	bindspnl:SetDraggable(true)
	bindspnl:SetWide(245)
	bindspnl:SetPos(bindspos.x, bindspos.y)
	bindspnl:DockPadding(0, -2, 0, 0) -- fix offset from top lol
end

local function CloseBindsPanel()
	if IsValid(bindspnl) then
		local x, y = bindspnl:GetPos()
		bindspos = {x=x, y=y}
		bindspnl:Remove()
	end
end

local spectators = {}

local function AddSpectator(ply)
	if !IsValid(spectatorspnl) or spectators[ply] then return end

	local label = vgui.Create("DLabel", bindspnl)
	label:Dock(TOP)
	label:DockMargin(4, 0, 0, 0)
	label:SetText(("%s %s"):format(ply:Nick(), ply:GetUserGroup()))

	InitLabel(nil, nil, label)

	spectators[ply] = label

	ResizePanel(spectatorspnl)
end

local function RemoveSpectator(ply)
	if !IsValid(spectatorspnl) or !spectators[ply] then return end

	local spectator = spectators[ply]

	if IsValid(spectator) then
		spectator:Remove()
		spectators[ply] = nil

		ResizePanel(spectatorspnl)
	end
end

local function OpenSpectatorsPanel()
	if IsValid(spectatorspnl) then return end

	spectatorspnl = Frame("spectators")
	spectatorspnl:SetTitle("")
	spectatorspnl:SetKeyboardInputEnabled(false)
	spectatorspnl:SetMouseInputEnabled(false)
	spectatorspnl:ShowCloseButton(false)
	spectatorspnl:SetDraggable(true)
	spectatorspnl:SetWide(245)
	spectatorspnl:SetPos(spectatorspos.x, spectatorspos.y)
	spectatorspnl:DockPadding(0, -2, 0, 0) -- fix offset from top lol
end

local function CloseSpectatorsPanel()
	if IsValid(spectatorspnl) then
		local x, y = spectatorspnl:GetPos()
		spectatorspos = {x=x, y=y}
		spectatorspnl:Remove()
	end
end

local function AddHook(name, func)
	local id = "GM_U" .. tostring(math.Round(CurTime() ^ 2) + mrandom(0, 1000000))
	hook.Add(name, id, func)
end

local fakemodels = {}
local NewFakeModel, UpdateFakeModel

do

function NewFakeModel(ply, group)
	local model = ClientsideModel(ply:GetModel(), group)
	model:SetNoDraw(true)

	local data = {
		model = model,
		ply = ply
	}

	fakemodels[#fakemodels + 1] = data

	return data
end

local function CopyPoseParam(name, from, to)
	local min, max = to:GetPoseParameterRange(from:LookupPoseParameter(name))
	if min then
		to:SetPoseParameter(name, min + (max - min) * from:GetPoseParameter(name))
	end
end

function UpdateFakeModel(model, angles)
	local mdl = model.model
	local ply = model.ply

	local ang
	if angles then
		ang = angles

		mdl:SetPoseParameter("aim_pitch", ang.p)
		mdl:SetPoseParameter("head_pitch", ang.p)
		mdl:SetPoseParameter("body_yaw", ang.y)
		mdl:SetPoseParameter("aim_yaw", 0)

		-- Fix legs
		local velocity = ply:GetVelocity()
		local velocityYaw = mNormalizeAng(ang.y - math.deg(math.atan2(velocity.y, velocity.x)))
		local playbackRate = ply:GetPlaybackRate()
		local moveX = math.cos(math.rad(velocityYaw)) * playbackRate
		local moveY = -math.sin(math.rad(velocityYaw)) * playbackRate

		mdl:SetPoseParameter("move_x", moveX)
		mdl:SetPoseParameter("move_y", moveY)
	else
		local lby = samoware.GetCurrentLBY(ply:EntIndex())
		ang = Angle(0, lby, 0)

		for i = 0, ply:GetNumPoseParameters() - 1 do
			local name = ply:GetPoseParameterName(i)
			CopyPoseParam(name, ply, mdl)
		end
	end

	mdl:SetPos(ply:GetPos())

	mdl:SetAngles(Angle(0, ang.y, 0))

	mdl:SetCycle(ply:GetCycle())
	mdl:SetSequence(ply:GetSequence())

	mdl:InvalidateBoneCache()
end

end

local chamsmat = CreateMaterial("SWBaseChams", "VertexLitGeneric", {
	["$basetexture"] = "color/white",
	["$model"] = 1
})

local chamsmat_transparent = CreateMaterial("SWBaseChams_transparent", "VertexLitGeneric", {
	["$basetexture"] = "color/white",
	["$model"] = 1,
	["$translucent"] = 1,
	["$vertexalpha"] = 1,
	["$vertexcolor"] = 1
})

local cmdmeta = FindMetaTable("CUserCmd")
local origCommandNumber = cmdmeta.CommandNumber
local origCreateMove = GAMEMODE.CreateMove

local _R = debug.getregistry()
local hook_Remove = hook.Remove

local function Unload()
	print("Unloading...")

	_R.bSendPacket = true

	for k,v in pairs(hook.GetTable()) do
		for k1, v1 in pairs(v) do
			if tostring(k1):StartWith("GM_") then
				print("Removing hook", k, k1)
				hook_Remove(k, k1)
			end
		end
	end

	GAMEMODE.CreateMove = origCreateMove

	-- Remove samoware hooks
	samoware.RemoveHook("PostFrameStageNotify", "samoware")

	for k, v in ipairs(fakemodels) do
		v.model:Remove()
		fakemodels[k] = nil
	end

	for k, v in ipairs(player.GetAll()) do
		v.sw_cur_pos = nil
		v.sw_prev_pos = nil
		v.sw_cur_simtime = nil
		v.sw_prev_simtime = nil
		v.sw_last_dormant = nil
		v.sw_aim_shots = nil
	end

	if IsValid(menupnl) then
		menupnl:Remove()
	end

	if IsValid(infopnl) then
		infopnl:Remove()
	end

	if IsValid(bindspnl) then
		bindspnl:Remove()
	end

	if IsValid(spectatorspnl) then
		spectatorspnl:Remove()
	end

	if IsValid(colorpicker) then
		colorpicker:Remove()
	end

	_R.SWUNLOAD = nil
end

if _R.SWUNLOAD then _R.SWUNLOAD() end
Unload()
_R.SWUNLOAD = Unload

GAMEMODE.CreateMove = function() end

do

local pressing_insert = false
local menu_opened = false
AddHook("Think", function()
	if input.IsKeyDown(KEY_END) then
		Unload()
		return
	end

	if IsValid(infopnl) then
		local x, y = infopnl:GetPos()
		infopos = {x=x, y=y}
	end

	if IsValid(bindspnl) then
		local x, y = bindspnl:GetPos()
		bindspos = {x=x, y=y}
	end

	if IsValid(spectatorspnl) then
		local x, y = spectatorspnl:GetPos()
		spectatorspos = {x=x, y=y}
	end

	pressing_insert = input.IsKeyDown(KEY_HOME)
	if pressing_insert and !menu_opened then
		menu_opened = true
		if !IsValid(menupnl) then
			OpenMenu()
		else
			CloseMenu()
		end
	elseif !pressing_insert then
		menu_opened = false
	end

	if settings.Visuals.EnabledI then
		OpenInfoPanel()

		if IsValid(menupnl) then
			infopnl:SetMouseInputEnabled(true)
		else
			infopnl:SetMouseInputEnabled(false)
		end
	else
		CloseInfoPanel()
	end

	if settings.Visuals.Binds then
		OpenBindsPanel()

		if IsValid(menupnl) then
			bindspnl:SetMouseInputEnabled(true)
		else
			bindspnl:SetMouseInputEnabled(false)
		end
	else
		CloseBindsPanel()
	end

	if settings.Visuals.Spectators then
		OpenSpectatorsPanel()

		if IsValid(menupnl) then
			spectatorspnl:SetMouseInputEnabled(true)
		else
			spectatorspnl:SetMouseInputEnabled(false)
		end

		for k, v in ipairs(player.GetAll()) do
			if v:GetObserverMode() != OBS_MODE_NONE and v:GetObserverTarget() == me then
				AddSpectator(v)
			else
				RemoveSpectator(v)
			end
		end
	else
		CloseSpectatorsPanel()
	end
end)

end

OpenMenu()

do
	local tickrate = tostring(math.Round(1 / engine.TickInterval()))
	RunConsoleCommand("cl_cmdrate", tickrate)
	RunConsoleCommand("cl_updaterate", tickrate)

	RunConsoleCommand("cl_interp", "0")
	RunConsoleCommand("cl_interp_ratio", "0")
end

local VelPredict, IsVisible
do

function VelPredict(vec, ply)
	local mode = settings.Aimbot.Prediction

	if mode == "None" then return vec end

	local lvel, tvel = me:GetVelocity(), ply:GetVelocity()

	if mode == "Engine" then
		return tvel == vector_origin and vec or vec + tvel * TICK_INTERVAL * RealFrameTime() - lvel * TICK_INTERVAL
	end

	if mode == "EngineC" then
		return vec + (tvel / 45 - lvel / 45)
	end

	if mode == "VelBase" then
		return vec + ((lvel - tvel) * (RealFrameTime() / (1 / TICK_INTERVAL)))
	end

	if mode == "Classic" then
		return vec - (lvel * TICK_INTERVAL)
	end
end

-- Cached trace struct, used in AutoWall and IsVisible
local traceStruct = {
	mask = MASK_SHOT,
	filter = me
}

local function AutoWall(dir, plyTarget)
	local weap = me:GetActiveWeapon()
	if !IsValid(weap) then return false end

	local class = weap:GetClass()
	local eyePos = me:EyePos()

	local function CW2Autowall()
		local normalmask = bor(CONTENTS_SOLID, CONTENTS_OPAQUE, CONTENTS_MOVEABLE, CONTENTS_DEBRIS, CONTENTS_MONSTER, CONTENTS_HITBOX, 402653442, CONTENTS_WATER)
		local wallmask = bor(CONTENTS_TESTFOGVOLUME, CONTENTS_EMPTY, CONTENTS_MONSTER, CONTENTS_HITBOX)

		traceStruct.start = eyePos
		traceStruct.endpos = eyePos + dir * weap.PenetrativeRange
		traceStruct.mask = normalmask

		local tr = TraceLine(traceStruct)

		if tr.Hit and !tr.HitSky then
			local canPenetrate, dot = weap:canPenetrate(tr, dir)

			if canPenetrate and dot > 0.26 then
				traceStruct.start = tr.HitPos
				traceStruct.endpos = tr.HitPos + dir * weap.PenStr * (weap.PenetrationMaterialInteraction[tr.MatType] or 1) * weap.PenMod
				traceStruct.mask = wallmask
				tr = TraceLine(traceStruct)

				traceStruct.start = tr.HitPos
				traceStruct.endpos = tr.HitPos + dir * 0.1
				traceStruct.mask = normalmask
				tr = TraceLine(traceStruct) -- run ANOTHER trace to check whether we've penetrated a surface or not

				if tr.Hit then return false end

				-- FireBullets
				traceStruct.start = tr.HitPos
				traceStruct.endpos = tr.HitPos + dir * 32768
				traceStruct.mask = MASK_SHOT
				tr = TraceLine(traceStruct)

				if settings.Aimbot["Force head"] then
					return tr.Entity == plyTarget and tr.HitGroup == HITGROUP_HEAD
				end

				return tr.Entity == plyTarget
			end
		end

		return false
	end

	local function SWBAutowall()
		local normalmask = bor(CONTENTS_SOLID, CONTENTS_OPAQUE, CONTENTS_MOVEABLE, CONTENTS_DEBRIS, CONTENTS_MONSTER, CONTENTS_HITBOX, 402653442, CONTENTS_WATER)
		local wallmask = bor(CONTENTS_TESTFOGVOLUME, CONTENTS_EMPTY, CONTENTS_MONSTER, CONTENTS_HITBOX)
		local penMod = {[MAT_SAND] = 0.5, [MAT_DIRT] = 0.8, [MAT_METAL] = 1.1, [MAT_TILE] = 0.9, [MAT_WOOD] = 1.2}

		traceStruct.start = eyePos
		traceStruct.endpos = eyePos + dir * weap.PenetrativeRange
		traceStruct.mask = normalmask
		local tr = TraceLine(traceStruct)

		if tr.Hit and !tr.HitSky then
			local dot = -dir:Dot(tr.HitNormal)

			if weap.CanPenetrate and dot > 0.26 then
				traceStruct.start = tr.HitPos
				traceStruct.endpos = tr.HitPos + dir * weap.PenStr * (penMod[tr.MatType] or 1) * weap.PenMod
				traceStruct.mask = wallmask
				tr = TraceLine(traceStruct)

				traceStruct.start = tr.HitPos
				traceStruct.endpos = tr.HitPos + dir * 0.1
				traceStruct.mask = normalmask
				tr = TraceLine(traceStruct) -- run ANOTHER trace to check whether we've penetrated a surface or not

				if tr.Hit then return false end

				-- FireBullets
				traceStruct.start = tr.HitPos
				traceStruct.endpos = tr.HitPos + dir * 32768
				traceStruct.mask = MASK_SHOT
				tr = TraceLine(traceStruct)

				if settings.Aimbot["Force head"] then
					return tr.Entity == plyTarget and tr.HitGroup == HITGROUP_HEAD
				end

				return tr.Entity == plyTarget
			end
		end

		return false
	end

	local function M9KAutowall()
		if !weap.Penetration then
			return false
		end

		local function BulletPenetrate(tr, bounceNum, damage)
			if damage < 1 then
				return false
			end

			local maxPenetration = 14
			local primaryAmmo = weap.Primary.Ammo
			if primaryAmmo == "SniperPenetratedRound" then -- .50 Ammo
				maxPenetration = 20
			elseif primaryAmmo == "pistol" then -- pistols
				maxPenetration = 9
			elseif primaryAmmo == "357" then -- revolvers with big ass bullets
				maxPenetration = 12
			elseif primaryAmmo == "smg1" then -- smgs
				maxPenetration = 14
			elseif primaryAmmo == "ar2" then -- assault rifles
				maxPenetration = 16
			elseif primaryAmmo == "buckshot" then -- shotguns
				maxPenetration = 5
			elseif primaryAmmo == "slam" then -- secondary shotguns
				maxPenetration = 5
			elseif primaryAmmo == "AirboatGun" then -- metal piercing shotgun pellet
				maxPenetration = 17
			end

			local isRicochet = false
			if primaryAmmo == "pistol" or primaryAmmo == "buckshot" or primaryAmmo == "slam" then
				isRicochet = true
			else
				/*
				TODO: Predict ricochetCoin?
				if weap.RicochetCoin == 1 then
					isRicochet = true
				elseif weap.RicochetCoin >= 2 then
					isRicochet = false
				end*/
			end

			if primaryAmmo == "SniperPenetratedRound" then
				isRicochet = true
			end

			local maxRicochet = 0
			if primaryAmmo == "SniperPenetratedRound" then -- .50 Ammo
				maxRicochet = 10
			elseif primaryAmmo == "pistol" then -- pistols
				maxRicochet = 2
			elseif primaryAmmo == "357" then -- revolvers with big ass bullets
				maxRicochet = 5
			elseif primaryAmmo == "smg1" then -- smgs
				maxRicochet = 4
			elseif primaryAmmo == "ar2" then -- assault rifles
				maxRicochet = 5
			elseif primaryAmmo == "buckshot" then -- shotguns
				maxRicochet = 0
			elseif primaryAmmo == "slam" then -- secondary shotguns
				maxRicochet = 0
			elseif primaryAmmo == "AirboatGun" then -- metal piercing shotgun pellet
				maxRicochet = 8
			end

			if tr.MatType == MAT_METAL and isRicochet and primaryAmmo != "SniperPenetratedRound" then
				return false
			end

			if bounceNum > maxRicochet then
				return false
			end

			local penetrationDir = tr.Normal * maxPenetration
			if tr.MatType == MAT_GLASS or tr.MatType == MAT_PLASTIC or tr.MatType == MAT_WOOD or tr.MatType == MAT_FLESH or tr.MatType == MAT_ALIENFLESH then
				penetrationDir = tr.Normal * (maxPenetration * 2) -- WAS 200
			end

			if tr.Fraction <= 0 then
				return false
			end

			traceStruct.start = tr.HitPos + penetrationDir
			traceStruct.endpos = tr.HitPos
			traceStruct.mask = MASK_SHOT
			local trace = TraceLine(traceStruct)

			if trace.StartSolid or trace.Fraction >= 1 then
				return false
			end

			traceStruct.start = trace.HitPos
			traceStruct.endpos = trace.HitPos + tr.Normal * 32768
			traceStruct.mask = MASK_SHOT
			penTrace = TraceLine(traceStruct)

			if settings.Aimbot["Force head"] then
				return penTrace.Entity == plyTarget and penTrace.HitGroup == HITGROUP_HEAD
			else
				if penTrace.Entity == plyTarget then
					return true
				end
			end

			local damageMulti = 0.5
			if weap.Primary.Ammo == "SniperPenetratedRound" then
				damageMulti = 1
			elseif tr.MatType == MAT_CONCRETE or tr.MatType == MAT_METAL then
				damageMulti = 0.3
			elseif tr.MatType == MAT_WOOD or tr.MatType == MAT_PLASTIC or tr.MatType == MAT_GLASS then
				damageMulti = 0.8
			elseif tr.MatType == MAT_FLESH or tr.MatType == MAT_ALIENFLESH then
				damageMulti = 0.9
			end

			if penTrace.MatType == MAT_GLASS then
				bounceNum = bounceNum - 1
			end

			return BulletPenetrate(penTrace, bounceNum + 1, damage * damageMulti)
		end

		traceStruct.start = eyePos
		traceStruct.endpos = eyePos + dir * 32768
		traceStruct.mask = MASK_SHOT
		local trace = TraceLine(traceStruct)

		return BulletPenetrate(trace, 0, weap.Primary.Damage)
	end

	if class:StartWith("cw_") then
		return CW2Autowall()
	elseif class:StartWith("m9k_") then
		return M9KAutowall()
	elseif class:StartWith("swb_") then
		return SWBAutowall()
	end

	return false
end

function IsVisible(pos, ply, extrselfticks, autowalldir)
	local start = me:EyePos()
	if extrselfticks then
		start = start + (me:GetVelocity() * TICK_INTERVAL) * extrselfticks
	end

	traceStruct.start = start
	traceStruct.endpos = pos
	traceStruct.mask = MASK_SHOT

	local tr = TraceLine(traceStruct)

	local visible = tr.Entity == ply or tr.Fraction == 1
	if !visible and autowalldir and settings.Aimbot.Autowalls then
		return AutoWall(autowalldir, ply)
	end

	if settings.Aimbot["Force head"] then
		return tr.HitGroup == HITGROUP_HEAD
	end

	return visible
end

end

local GetBone, FindBone

do
/*--------------JIT VECTOR & ANGLE--------------*/
local function JitVector(x, y, z)
	return {x = x, y = y, z = z}
end

local function JitAngle(p, y, r)
	return {p = p, y = y, r = r}
end

local function JitVector_ToEngine(vec)
	return Vector(vec.x, vec.y, vec.z)
end

local function JitVector_Copy(vec)
	return JitVector(vec.x, vec.y, vec.z)
end

local function JitVector_Add(vec1, vec2)
	return JitVector(vec1.x + vec2.x, vec1.y + vec2.y, vec1.z + vec2.z)
end

local function JitVector_Sub(vec1, vec2)
	return JitVector(vec1.x - vec2.x, vec1.y - vec2.y, vec1.z - vec2.z)
end

local function JitVector_AddInPlace(dest, src)
	dest.x = dest.x + src.x
	dest.y = dest.y + src.y
	dest.z = dest.z + src.z
	return dest
end

local function JitVector_SubInPlace(dest, src)
	dest.x = dest.x - src.x
	dest.y = dest.y - src.y
	dest.z = dest.z - src.z
	return dest
end

local function JitVector_Extend(vec, value)
	return JitVector(vec.x * value, vec.y * value, vec.z * value)
end

local function JitVector_ExtendInPlace(vec, value)
	vec.x = vec.x * value
	vec.y = vec.y * value
	vec.z = vec.z * value
	return vec
end

local function JitVector_Length2D(vec)
	return math.sqrt(vec.x * vec.x + vec.y * vec.y)
end

local function JitVector_DistToSqr(vec1, vec2)
	local dx = vec1.x - vec2.x
	local dy = vec1.y - vec2.y
	local dz = vec1.z - vec2.z
	return dx * dx + dy * dy + dz * dz
end

local function JitVector_Distance(vec1, vec2)
	return math.sqrt(JitVector_DistToSqr(vec1, vec2))
end

local function JitVector_Angle(vec)
	local angle = JitAngle(0, 0, 0)

	if vec.y == 0 and vec.x == 0 then
		angle.y = 0

		if vec.z > 0 then
			angle.p = 270
		else
			angle.p = 90
		end
	else
		angle.y = math.deg(math.atan2(vec.y, vec.x))
		if angle.y < 0 then
			angle.y = angle.y + 360
		end

		angle.p = math.deg(math.atan2(-vec.z, JitVector_Length2D(vec)))
		if angle.p < 0 then
			angle.p = angle.p + 360
		end
	end

	return angle
end

local function JitAngle_Copy(angle)
	return JitAngle(angle.p, angle.y, angle.r)
end

local function JitAngle_ToEngine(angle)
	return Angle(angle.p, angle.y, angle.r)
end

local function JitAngle_Forward(angle)
	local radp = math.rad(angle.p)
	local rady = math.rad(angle.y)

	local sp, cp = math.sin(radp), math.cos(radp)
	local sy, cy = math.sin(rady), math.cos(rady)

	return JitVector(cp * cy, cp * sy, -sp)
end

local function JitAngle_Right(angle)
	local radp = math.rad(angle.p)
	local rady = math.rad(angle.y)
	local radr = math.rad(angle.r)

	local sp, cp = math.sin(radp), math.cos(radp)
	local sy, cy = math.sin(rady), math.cos(rady)
	local sr, cr = math.sin(radr), math.cos(radr)

	return JitVector(-1 * sr * sp * cy + -1 * cr * -sy,
					-1 * sr * sp * sy + -1 * cr * cy,
					-1 * sr * cp)
end

local function JitAngle_Up(angle)
	local radp = math.rad(angle.p)
	local rady = math.rad(angle.y)
	local radr = math.rad(angle.r)

	local sp, cp = math.sin(radp), math.cos(radp)
	local sy, cy = math.sin(rady), math.cos(rady)
	local sr, cr = math.sin(radr), math.cos(radr)

	return JitVector(cr * sp * cy + -sr * -sy,
					cr * sp * sy + -sr * cy,
					cr * sp)
end
/*--------------JIT VECTOR & ANGLE--------------*/

local bones = {
	["Head"] = "ValveBiped.Bip01_Head1",
	["Pelvis"] = "ValveBiped.Bip01_Pelvis",
	["Pelvis2"] = "ValveBiped.Bip01_Pelvis2",
	["Thigh"] = "ValveBiped.Bip01_L_Thigh"
}

local fixedBoneCache = {}
function FindBone(ply, name)
	if fixedBoneCache[ply] and fixedBoneCache[ply][name] then
		return fixedBoneCache[ply][name]
	end

	local boneid = ply:LookupBone(bones[name] or name)
	if !boneid then
		if !fixedBoneCache[ply] then
			fixedBoneCache[ply] = {}
		end

		local numBones = ply:GetBoneCount()
		for i = 0, numBones - 1 do
			local boneName = ply:GetBoneName(i)
			if boneName:find(name) then
				fixedBoneCache[ply][name] = i
				return i
			end
		end
	end

	return boneid
end

local safetyRanking = {
	"Pelvis",
	"Spine",
	"Clavicle",
	"UpperArm",
	"Head",
	"Neck",
	"Forearm",
	"Hand",
	"Thigh",
	"Calf",
	"Foot",
	"Toe"
}

local damageRanking = {
	"Head",
	"Neck",
	"Spine",
	"Pelvis",
	"Clavicle",
	"UpperArm",
	"Thigh"
}

local boneRankingCache = {
	[safetyRanking] = {},
	[damageRanking] = {}
}

local function GetBoneRanking(ply, tbl, bone)
	local boneName = ply:GetBoneName(bone)
	local rankingCache = boneRankingCache[tbl]
	if rankingCache[boneName] then
		return rankingCache[boneName]
	end

	for i = 1, #tbl do
		local name = tbl[i]
		if boneName:find(name, 1, true) then
			rankingCache[boneName] = i
			return i
		end
	end

	return 1000
end

-- JITTED
local function InsertMultiPoint(outTbl, boneId, boneCenter, boneAng, boneSize)
	if !settings.Aimbot.MultiPoint then
		outTbl[#outTbl + 1] = {boneId, boneCenter, boneSize}
		return
	end

	outTbl[#outTbl + 1] = {boneId, boneCenter, boneSize}

	local factor = settings.Aimbot.MultiPointFactor * msqrt(boneSize) / msqrt(2) / 2
	local forward, right, up = JitAngle_Forward(boneAng), JitAngle_Right(boneAng), JitAngle_Up(boneAng)
	--boneAng:Up(), boneAng:Forward(), boneAng:Right()

	local positions = {
		JitVector_Add(boneCenter, JitVector_Extend(up, factor)),
		JitVector_Sub(boneCenter, JitVector_Extend(up, factor)),

		JitVector_Add(boneCenter, JitVector_Extend(forward, factor)),
		JitVector_Sub(boneCenter, JitVector_Extend(forward, factor)),

		JitVector_Add(boneCenter, JitVector_Extend(right, factor)),
		JitVector_Sub(boneCenter, JitVector_Extend(right, factor)),
	}

	for i = 1, 6 do
		local pos = positions[i]
		outTbl[#outTbl + 1] = {
			boneId,
			pos,
			JitVector_DistToSqr(pos, boneCenter) --pos:DistToSqr(boneCenter)
		}
	end
end

function GetBone(ply, modeOverride, boneOverride)
	local boneId = FindBone(ply, boneOverride and boneOverride or settings.Aimbot.Bone)
	if !boneId then
		-- Select random bone
		local numBones = ply:GetBoneCount()
		boneId = mrandom(0, numBones - 1)
	end

	local min, max = ply:GetHitBoxBounds(0, 0)
	if !min or !max then
		return {ply:GetBonePosition(boneId)}
	end

	local bone, ang = ply:GetBonePosition(boneId)
	min:Rotate(ang)
	max:Rotate(ang)

	local pos = bone + ((min + max) * 0.5)

	local mode = modeOverride or settings.Aimbot.BoneMode
	if mode == "Center" then
		local boneMat = ply:GetBoneMatrix(boneId)

		if boneMat then
			local bonePos, boneAng = boneMat:GetTranslation(), boneMat:GetAngles()

			-- Find bone hitbox
			local multiPointResult = {}
			for hitboxSet = 0, ply:GetHitboxSetCount() - 1 do
				for hitbox = 0, ply:GetHitBoxCount(hitboxSet) - 1 do
					local hitboxBoneId = ply:GetHitBoxBone(hitbox, hitboxSet)
					if boneId != hitboxBoneId then continue end

					local mins, maxs = ply:GetHitBoxBounds(hitbox, hitboxSet)
					local boneSize = JitVector_DistToSqr(maxs, mins)
					mins:Rotate(boneAng)
					maxs:Rotate(boneAng)

					local boneCenterLocal = JitVector_AddInPlace(mins, maxs)
					JitVector_ExtendInPlace(boneCenterLocal, 0.5)

					local boneCenter = JitVector_AddInPlace(bonePos, boneCenterLocal)
					boneCenter = VelPredict(boneCenter, ply)

					InsertMultiPoint(multiPointResult, boneId, boneCenter, boneAng, boneSize)

					-- Return only bone positions
					for i = 1, #multiPointResult do
						-- debugoverlay.Cross(multiPoints[i][2], 1, 0.1, color_white, true)
						multiPointResult[i] = JitVector_ToEngine(multiPointResult[i][2])
					end

					return multiPointResult
				end
			end

			multiPointResult[1] = VelPredict(bonePos, ply)

			return multiPointResult
		end
	elseif mode == "HitScan" then
		local bones = {}
		for hitboxSet = 0, ply:GetHitboxSetCount() - 1 do
			for hitbox = 0, ply:GetHitBoxCount(hitboxSet) - 1 do
				local boneId = ply:GetHitBoxBone(hitbox, hitboxSet)
				local boneMat = ply:GetBoneMatrix(boneId)
				if !boneMat then continue end

				local bonePos, boneAng = boneMat:GetTranslation(), boneMat:GetAngles()
				local mins, maxs = ply:GetHitBoxBounds(hitbox, hitboxSet)
				local boneSize = JitVector_DistToSqr(maxs, mins)

				mins:Rotate(boneAng)
				maxs:Rotate(boneAng)

				local boneCenterLocal = JitVector_AddInPlace(mins, maxs)
				JitVector_ExtendInPlace(boneCenterLocal, 0.5)

				local boneCenter = JitVector_AddInPlace(bonePos, boneCenterLocal)
				boneCenter = VelPredict(boneCenter, ply)

				InsertMultiPoint(bones, boneId, boneCenter, boneAng, boneSize)
			end
		end

		local hitScanMode = settings.Aimbot.HitScanMode
		if hitScanMode == "Damage" then
			table.sort(bones, function(a, b)
				local rankA = GetBoneRanking(ply, damageRanking, a[1])
				local rankB = GetBoneRanking(ply, damageRanking, b[1])

				return rankA < rankB
			end)
		elseif hitScanMode == "Scale" then
			table.sort(bones, function(a, b)
				return a[3] > b[3]
			end)
		elseif hitScanMode == "Safety" then
			table.sort(bones, function(a, b)
				local rankA = GetBoneRanking(ply, safetyRanking, a[1])
				local rankB = GetBoneRanking(ply, safetyRanking, b[1])

				return rankA < rankB
			end)
		end

		-- Return only bone positions
		for i = 1, #bones do
			bones[i] = JitVector_ToEngine(bones[i][2])
		end

		return bones
	end

	return { pos }
end

end

local GetPlayers

do
function GetPlayers(mode, extrselfticks, extrplyticks, checkvis)
	local selfpos = me:EyePos()
	if extrselfticks then
		selfpos = selfpos + (me:GetVelocity() * TICK_INTERVAL) * extrselfticks
	end

	local tbl = {}
	local plys = player.GetAll()
	for i = 1, #plys do
		local ply = plys[i]
		if ply == me then continue end
		if ply:IsDormant() or !ply:Alive() then continue end
		if settings.Aimbot["Ignore bots"] and ply:IsBot() then continue end
		if settings.Aimbot["Ignore team"] and ply:Team() == me:Team() then continue end
		if settings.Aimbot["Ignore friends"] and ply:GetFriendStatus() == "friend" then continue end
		if settings.Aimbot["Ignore admins"] and ply:IsAdmin() then continue end

		if settings.Aimbot["Ignore legits"] then
			local pitch = ply:EyeAngles().p
			if pitch > -88 and pitch < 88 then continue end
		end

		if checkvis then
			local bone = GetBone(ply, "Center")[1]
			if !bone then continue end

			local dir = me:GetShootPos() - bone
			dir:Normalize()
			if !IsVisible(bone, ply, extrselfticks, dir) then
				continue
			end
		end

		local pos = ply:GetPos()
		if extrplyticks then
			pos = pos + (ply:GetVelocity() * TICK_INTERVAL) * extrplyticks
		end

		tbl[#tbl + 1] = {ply, pos}
	end

	if mode == "My pos" then
		table.sort(tbl, function(a, b)
			return (a[2] - selfpos):LengthSqr() < (b[2] - selfpos):LengthSqr()
		end)
	elseif mode == "Crosshair 2D" then
		-- table.sort(tbl, function(a, b)
		-- 	local as, bs = a[2]:ToScreen(), b[2]:ToScreen()
		-- 	local sw, sh = SCRW * 0.5, SCRH * 0.5
		-- 	return Vector(sw - as.x, sh - as.y, 0):LengthSqr() < Vector(sw - bs.x, sh - bs.y, 0):LengthSqr()
		-- end)

		local wCenter, hCenter = SCRW * 0.5, SCRH * 0.5
		table.sort(tbl, function(a, b)
			local aScr, bScr = a[2]:ToScreen(), b[2]:ToScreen()

			local aDist
			do
				local dx = wCenter - aScr.x
				local dy = hCenter - aScr.y
				aDist = dx * dx + dy * dy
			end

			local bDist
			do
				local dx = wCenter - bScr.x
				local dy = hCenter - bScr.y
				bDist = dx * dx + dy * dy
			end

			return aDist < bDist
		end)
	elseif mode == "Crosshair" then
		local hitpos = TraceLine({
			start = selfpos,
			endpos = selfpos + me:GetForward() * 10000,
			filter = me
		}).HitPos

		table.sort(tbl, function(a, b)
			return (a[2] - hitpos):LengthSqr() < (b[2] - hitpos):LengthSqr()
		end)
	end

	if #tbl == 0 then return end

	return tbl
end

end

local UpdateKeybinds

do

local function IsKeyDown(key)
	if key >= MOUSE_FIRST then
		return input.IsMouseDown(key)
	end

	return input.IsKeyDown(key)
end

local bindlisteners = {}
local keystates = {}
local function ListenKeybind(tab, name)
	bindlisteners[#bindlisteners + 1] = {tab, name}
	keybinds[tab] = keybinds[tab] or {}
	keybinds[tab][name] = false

	keystates[tab] = keystates[tab] or {}
	keystates[tab][name] = false
end

for tab, tbl in pairs(settings) do
	for setting, value in pairs(tbl) do
		if istable(value) and value.key and value.mode then
			ListenKeybind(tab, setting)
		end
	end
end

function UpdateKeybinds()
	if IsValid(menupnl) or gui.IsGameUIVisible() or vgui.GetKeyboardFocus() then return end

	for k, v in ipairs(bindlisteners) do
		local tab, name = v[1], v[2]
		local data = settings[tab][name]
		local mode = data.mode

		if mode == "Always" then
			keybinds[tab][name] = true
		elseif mode == "Hold" then
			keybinds[tab][name] = IsKeyDown(data.key)
		elseif mode == "Toggle" then
			local pressed = IsKeyDown(data.key)
			if pressed and keystates[tab][name] then
				continue
			elseif !pressed then
				keystates[tab][name] = false
				continue
			end

			keystates[tab][name] = true
			keybinds[tab][name] = !keybinds[tab][name]
		elseif mode == "Press" then
			local pressed = IsKeyDown(data.key)
			if pressed and keystates[tab][name] then
				keybinds[tab][name] = false
				continue
			elseif !pressed then
				keystates[tab][name] = false
				continue
			end

			keystates[tab][name] = true
			keybinds[tab][name] = true
		end

		if keybinds[tab][name] then
			AddActiveBind(tab, name)
		else
			RemoveActiveBind(tab, name)
		end
	end
end

end

local FixMove
do

local c_sv_noclipspeed = GetConVar("sv_noclipspeed")
local c_sv_specspeed = GetConVar("sv_specspeed")
function FixMove(cmd, aWishDir)
	local factor = 1
	if me:GetObserverMode() == OBS_MODE_ROAMING then
		factor = c_sv_specspeed:GetFloat()
	else
		factor = c_sv_noclipspeed:GetFloat()
	end

	if cmd:KeyDown(IN_SPEED) then
		factor = factor * 0.5
	end

	local aRealDir = cmd:GetViewAngles()
	aRealDir:Normalize()

	local vRealF = aRealDir:Forward()
	local vRealR = aRealDir:Right()
	vRealF.z = 0
	vRealR.z = 0

	vRealF:Normalize()
	vRealR:Normalize()

	aWishDir:Normalize()

	local vWishF = aWishDir:Forward()
	local vWishR = aWishDir:Right()
	vWishF.z = 0
	vWishR.z = 0

	vWishF:Normalize()
	vWishR:Normalize()

	local forwardmove = cmd:GetForwardMove() * factor
	local sidemove = cmd:GetSideMove() * factor
	local upmove = cmd:GetUpMove() * factor

	local vWishVel = vWishF * forwardmove + vWishR * sidemove
	vWishVel.z = vWishVel.z + upmove

	local a, b, c, d, e, f = vRealF.x, vRealR.x, vRealF.y, vRealR.y, vRealF.z, vRealR.z
	local u, v, w = vWishVel.x, vWishVel.y, vWishVel.z
	local flDivide = (b * c - a * d) * factor

	local x = -(d * u - b * v) / flDivide
	local y = (c * u - a * v) / flDivide
	local z = (a * (f * v - d * w) + b * (c * w - e * v) + u * (d * e - c * f)) / flDivide

	x = math.Clamp(x, -10000, 10000)
	y = math.Clamp(y, -10000, 10000)
	z = math.Clamp(z, -10000, 10000)

	cmd:SetForwardMove(x)
	cmd:SetSideMove(y)
	cmd:SetUpMove(z)
end

end

local function UpdateSilentAngles(cmd)
	local sensivity = 1.28

	local dx, dy = cmd:GetMouseX(), cmd:GetMouseY()
	silentangles = silentangles or cmd:GetViewAngles()
	silentangles.p = silentangles.p + dy * 0.023 * sensivity
	silentangles.y = silentangles.y + dx * -0.023 * sensivity

	silentangles.p = mClamp(silentangles.p, -89, 89)
	silentangles.y = silentangles.y % 360
	silentangles.r = 0

	if cmd:CommandNumber() == 0 then return end
	cmd:SetViewAngles(silentangles)
end

local GetServerTime = samoware.GetServerTime

local reloadsequences = {
	[ACT_VM_RELOAD] = true,
	[ACT_VM_RELOAD_SILENCED] = true,
	[ACT_VM_RELOAD_DEPLOYED] = true,
	[ACT_VM_RELOAD_IDLE] = true,
	[ACT_VM_RELOAD_EMPTY] = true,
	[ACT_VM_RELOADEMPTY] = true,
	[ACT_VM_RELOAD_M203] = true,
	[ACT_VM_RELOAD_INSERT] = true,
	[ACT_VM_RELOAD_INSERT_PULL] = true,
	[ACT_VM_RELOAD_END] = true,
	[ACT_VM_RELOAD_END_EMPTY] = true,
	[ACT_VM_RELOAD_INSERT_EMPTY] = true,
	[ACT_VM_RELOAD2] = true
}

local function CanShoot(cmd)
	local weap = me:GetActiveWeapon()
	if !IsValid(weap) or weap:Clip1() == 0 then return false end

	local seq = weap:GetSequence()
	if reloadsequences[seq] then return false end

	local delay = 0
	if settings.Aimbot.ShootDelay then
		delay = settings.Aimbot.Delay / 1000
	end

	return (GetServerTime(cmd) - delay) >= weap:GetNextPrimaryFire()
end

local function PredictSpread(cmd, ang, spread)
	if settings.HvH.Dickwrap then
		spread = (spread.x + spread.y) / 2
		local spreadDir = samoware.PredictSpread(cmd, ang, spread)
		local newAngles = ang + spreadDir:Angle()
		newAngles:Normalize()

		return newAngles
	end

	return ang
end

local weapcones = {}
local function RemoveSpread(cmd, ang)
	local weap = me:GetActiveWeapon()
	local class = weap:GetClass()

	if class:StartWith("cw_") then
		-- local memevec = Angle(0, cmd:CommandNumber() % 2 * 180, 0):Forward()

		local function CalculateSpread()
			if not weap.AccuracyEnabled then
				return
			end

			local aim = ang:Forward()
			local CT = CurTime()
			local dt = TICK_INTERVAL --FrameTime()

			if !me.LastView then
				me.LastView = aim
				me.ViewAff = 0
			else
				me.ViewAff = LerpCW20(dt * 10, me.ViewAff, (aim - me.LastView):Length() * 0.5)
				me.LastView = aim
			end

			local baseCone, maxSpreadMod = weap:getBaseCone()
			weap.BaseCone = baseCone

			if me:Crouching() then
				weap.BaseCone = weap.BaseCone * weap:getCrouchSpreadModifier()
			end

			weap.CurCone = weap:getFinalSpread(me:GetVelocity():Length2D(), maxSpreadMod)

			if CT > weap.SpreadWait then
				weap.AddSpread = mClamp(weap.AddSpread - 0.5 * weap.AddSpreadSpeed * dt, 0, weap:getMaxSpreadIncrease(maxSpreadMod))
				weap.AddSpreadSpeed = mClamp(weap.AddSpreadSpeed + 5 * dt, 0, 1)
			end
		end

		-- samoware.SetContextVector(cmd, memevec)

		CalculateSpread()

		local cone = weap.CurCone
		if !cone then return ang end

		if me:Crouching() then
			cone = cone * 0.85
		end

		math.randomseed(cmd:CommandNumber())
		ang = ang - Angle(mRand(-cone, cone), mRand(-cone, cone), 0) * 25
	elseif class:StartWith("arccw_") then
		local angDir = ang:Forward()

		local seed1 = weap:GetBurstCount()
		local seed2 = !game.SinglePlayer() and cmd:CommandNumber() or CurTime()

		local randSeed = util.SharedRandom(seed1, -1337, 1337, seed2) * (weap:EntIndex() % 30241)
		math.randomseed(math.Round(randSeed))

		local spread = ArcCW.MOAToAcc * weap:GetBuff("AccuracyMOA")
		local disp = weap:GetDispersion() * ArcCW.MOAToAcc / 10

		angDir:Rotate(Angle(0, -ArcCW.StrafeTilt(weap), 0))
		angDir = angDir - VectorRand() * disp

		local randSeed = util.SharedRandom(1, -1337, 1337, seed2) * (weap:EntIndex() % 30241)
		math.randomseed(math.Round(randSeed))
		angDir = angDir - VectorRand() * spread

		return angDir:Angle()
	elseif class:StartWith("swb_") then
		local function CalculateSpread()
			local vel = me:GetVelocity():Length()
			local dir = ang:Forward()

			if !me.LastView then
				me.LastView = dir
				me.ViewAff = 0
			else
				me.ViewAff = Lerp(0.25, me.ViewAff, (dir - me.LastView):Length() * 0.5)
				--  me.LastView = dir
			end

			if weap.dt.State == SWB_AIMING then
				weap.BaseCone = weap.AimSpread

				if weap.Owner.Expertise then
					weap.BaseCone = weap.BaseCone * (1 - weap.Owner.Expertise["steadyaim"].val * 0.0015)
				end
			else
				weap.BaseCone = weap.HipSpread

				if weap.Owner.Expertise then
					weap.BaseCone = weap.BaseCone * (1 - weap.Owner.Expertise["wepprof"].val * 0.0015)
				end
			end

			if me:Crouching() then
				weap.BaseCone = weap.BaseCone * (weap.dt.State == SWB_AIMING and 0.9 or 0.75)
			end

			local curCone = weap.BaseCone + weap.AddSpread + (vel / 10000 * weap.VelocitySensitivity) * (weap.dt.State == SWB_AIMING and weap.AimMobilitySpreadMod or 1) + me.ViewAff
			weap.CurCone = mClamp(curCone, 0, 0.09 + weap.MaxSpreadInc)

			if CurTime() > weap.SpreadWait then
				weap.AddSpread = mClamp(weap.AddSpread - 0.005 * weap.AddSpreadSpeed, 0, weap.MaxSpreadInc)
				weap.AddSpreadSpeed = mClamp(weap.AddSpreadSpeed + 0.05, 0, 1)
			end
		end

		CalculateSpread()

		local cone = weap.CurCone
		if !cone then return ang end

		if me:Crouching() then
			cone = cone * 0.85
		end

		math.randomseed(cmd:CommandNumber())
		ang = ang - Angle(mRand(-cone, cone), mRand(-cone, cone), 0) * 25
	elseif weapcones[class] then
		local spread = weapcones[class]
		return PredictSpread(cmd, ang, spread)
	end

	return ang
end

local function RemoveRecoil(ang)
	local weap = me:GetActiveWeapon()
	local class = weap:GetClass()

	if class:StartWith("m9k_") then
		return ang
	else
		ang = ang - me:GetViewPunchAngles()
	end

	return ang
end

local function TIME_TO_TICKS(time)
	return math.floor(0.5 + time / TICK_INTERVAL)
end

local function TICKS_TO_TIME(ticks)
	return ticks * TICK_INTERVAL
end

-- https://github.com/ValveSoftware/source-sdk-2013/blob/0d8dceea4310fde5706b3ce1c70609d72a38efdf/mp/src/game/server/gameinterface.cpp#L2844
local function GetLerpTime()
	if GetConVar("cl_interpolate"):GetInt() == 0 then return 0 end

	local lerpRatio = GetConVar("cl_interp_ratio"):GetFloat()
	if lerpRatio == 0 then
		lerpRatio = 1
	end

	local lerpAmount = GetConVar("cl_interp"):GetFloat()
	if samoware.GetManipulateInterp() then
		lerpAmount = samoware.GetTargetInterp()
	end

	local updateRate = GetConVar("cl_updaterate"):GetFloat()

	return mmax(lerpAmount, lerpRatio / updateRate)
end

local Aimbot

do

function GetAimbotTarget(cmd)
	local plys = GetPlayers(settings.Aimbot.Mode)
	if !plys then return end

	if settings.HvH.Backshoot then
		for i = 1, #plys do
			local ply = plys[i][1]

			-- TODO: IMPROVE THIS SHITCODE
			if ply.sw_backshoot_data and ply.sw_prev_pos:DistToSqr(ply.sw_cur_pos) < 4096 then
				local function canBacktrack()
					local correct = samoware.GetLatency(0)
					correct = correct + GetLerpTime()
					if correct > 1 then
						return false
					end

					local serverArriveTick = samoware.GetServerTime(cmd) + samoware.GetLatency(0) + samoware.GetLatency(1)
					local diff = serverArriveTick - ply.sw_backshoot_data.simTime
					if diff > settings.HvH.BacktrackAmount / 1000 then
						return false
					end

					return true
				end

				if canBacktrack() then
					-- Can backshoot
					local aimAng = (ply.sw_backshoot_data.headPos - me:EyePos()):Angle()
					local tr = TraceLine({
						start = me:EyePos(),
						endpos = ply.sw_backshoot_data.headPos,
						filter = me,
						mask = MASK_SHOT
					})

					if !tr.Hit or tr.Entity == ply then
						return ply, aimAng, true
					end
				end
			end
		end
	end

	for i = 1, #plys do
		local ply = plys[i][1]

		local bones = GetBone(ply)
		local bonePos
		for j = 1, #bones do
			bonePos = bones[j]

			local aimAng = (bonePos - me:EyePos()):Angle()
			if IsVisible(bonePos, ply, nil, aimAng:Forward()) then
				return ply, aimAng, false
			end
		end
	end
end

function Aimbot(cmd)
	local aimPly, aimAngs, isBackshoot = GetAimbotTarget(cmd)
	aimply = aimPly

	if settings.HvH.ForceBackshoot and !isBackshoot then return false end

	if !aimAngs then return false end

	aimAngs:Normalize()

	local angnocomp = Angle(aimAngs)

	if settings.Aimbot.FOV then
		local fov = settings.Aimbot["FOV cone"]

		local view = settings.Aimbot.Silent and silentangles or cmd:GetViewAngles()
		local ang = aimAngs - view

		ang:Normalize()

		ang = msqrt(ang.x * ang.x + ang.y * ang.y)

		if ang > fov then
			return
		end
	end

	if settings.Aimbot.Nospread then
		aimAngs = RemoveSpread(cmd, aimAngs)
	end

	if settings.Aimbot.Norecoil then
		aimAngs = RemoveRecoil(aimAngs)
	end

	local weap = me:GetActiveWeapon():GetClass()
	if !weap:StartWith("cw_") and !weap:StartWith("swb_") and settings.Aimbot["Invert on shot"] then
		aimAngs = Angle(-aimAngs.p - 180, aimAngs.y + 180, 0)
	end

	if settings.HvH.Backshoot then
		samoware.SetManipulateInterp(true)
	else
		samoware.SetManipulateInterp(false)
	end

	if settings.HvH.LagFix then
		if isBackshoot then
			local targetTime = aimPly.sw_backshoot_data.simTime
			local timeOffset = samoware.GetServerTime(cmd) - targetTime

			-- Check if we can backtrack without cl_interp
			local serverArriveTick = samoware.GetServerTime(cmd) + samoware.GetLatency(0) + samoware.GetLatency(1)
			local diff = serverArriveTick - aimPly.sw_backshoot_data.simTime
			if diff < 0.2 then
				samoware.SetTargetInterp(0)
				local tick = TIME_TO_TICKS(targetTime + GetLerpTime())
				samoware.SetCommandTick(cmd, tick)
			else
				samoware.SetTargetInterp(samoware.GetServerTime(cmd) - targetTime)

				local tick = TIME_TO_TICKS(samoware.GetServerTime(cmd))
				samoware.SetCommandTick(cmd, tick - 1)
			end
		else
			local simTime = samoware.GetSimulationTime(aimPly:EntIndex())

			if settings.HvH.Forwardtrack then
				local predTime = (samoware.GetLatency(0) + samoware.GetLatency(1)) * settings.HvH.ForwardtrackAmount
				simTime = simTime + predTime
			end

			local tick = TIME_TO_TICKS(simTime + GetLerpTime())
			samoware.SetCommandTick(cmd, tick)
		end
	end

	aimPly.sw_aim_shots = (aimPly.sw_aim_shots or 0) + 1

	if !settings.HvH["FakeLag on shoot"] and (!settings.HvH.FakeDuck or !keybinds.HvH.FakeDuckKey) then
		bSendPacket = true
	end

	if settings.Aimbot.PSilent and !weap:StartWith("cw_") and !weap:StartWith("arccw_") then
		local norm = angnocomp:Forward()
		local cnorm = samoware.CompressNormal(norm)

		-- make sure our compressed vector will hit aimPly
		if !IsVisible(me:EyePos() + cnorm * 32768, aimPly, nil, cnorm) then
			if settings.Aimbot["Force PSilent"] then
				return false
			else
				cmd:SetViewAngles(aimAngs)
			end
		end

		samoware.SetContextVector(cmd, aimAngs:Forward())
	else
		cmd:SetViewAngles(aimAngs)
	end

	if settings.Aimbot.Autoshoot then
		cmd:SetButtons(bor(cmd:GetButtons(), IN_ATTACK))
	end

	return true
end

end

local function GetPlayerYawInv()
	if !IsValid(aaply) then
		return silentangles.y
	end

	return mNormalizeAng((VelPredict(aaply:GetPos(), aaply) - me:EyePos()):Angle().y + 180)
end

local FakeLag, CheckPeeking, FakeLagInAir, ticksUntilSend

do

ticksUntilSend = 0
function FakeLag(cmd)
	bSendPacket = false
	if ticksUntilSend <= 0 then
		if settings.HvH["Randomize choke"] then
			local numChokes = settings.HvH["FakeLag choke"]
			local randomChokes = mrandom(0, 21 - numChokes)
			ticksUntilSend = numChokes + randomChokes
		else
			ticksUntilSend = settings.HvH["FakeLag choke"]
		end

		bSendPacket = true
	else
		ticksUntilSend = ticksUntilSend - 1
	end
end

local peeking, peeked = false, false

local function FakeLagOnPeek(cmd)
	ticksUntilSend = 21 - chokes - 1
	if chokes >= 20 then
		peeked = true
		peeking = false
		bSendPacket = true
		return
	end
end

local function WarpOnPeek()
	ShiftTickbase()

	peeked = true
	peeking = false
end

function CheckPeeking()
	local plys
	for extr = 1, 8 do
		plys = GetPlayers("My pos", extr, 1, true)
		if plys then break end
	end

	if plys and !peeking and !peeked then
		peeking = true
		peeked = false
	elseif !plys then
		peeking = false
		peeked = false
	end

	if peeking and !peeked then
		if !shiftingtickbase and tickbaseshiftcharge and settings.HvH["Warp on peek"] then
			WarpOnPeek()
		elseif settings.HvH["FakeLag on peek"] then
			FakeLagOnPeek()
		end
	end
end

function FakeLagInAir(cmd)
	if !me:IsOnGround() and chokes < 14 then
		bSendPacket = false
	end
end

end

local YawAntiAim
do

local function ClosestPointOnRay(target, rayStart, rayEnd)
	local to = target - rayStart
	local dir = rayEnd - rayStart
	local length = dir:Length()
	dir:Normalize()

	local rangeAlong = dir:Dot(to)
	if rangeAlong < 0 then
		return rayStart
	end

	if rangeAlong > length then
		return rayEnd
	end

	return rayStart + dir * rangeAlong
end

local impacts = {}
samoware.AddHook("OnImpact", "samoware_aa", function(data)
	local bestPlayer = nil
	local bestDistance = math.huge
	for _, ply in ipairs(player.GetAll()) do
		local distance = ply:EyePos():DistToSqr(data.m_vStart)
		if distance < bestDistance then
			bestPlayer = ply
			bestDistance = distance
		end
	end

	local attacker = bestPlayer

	local headId = FindBone(me, "Head")
	local headMat = me:GetBoneMatrix(headId)
	local headPos
	if headMat then
		headPos = headMat:GetTranslation()
	else
		headPos = me:GetBonePosition(headId)
	end

	if headPos then
		local nearestHeadPos = ClosestPointOnRay(headPos, data.m_vStart, data.m_vOrigin)
		local headPosDiff = headPos - nearestHeadPos
		local distanceToHead = headPosDiff:Length()
		if distanceToHead < 64 then
			local ang = -math.deg(math.atan2(headPosDiff.y, headPosDiff.x)) + realangles.y
			ang = mNormalizeAng(ang)

			attacker.sw_anti_bruteforce_angle = ang > 0 and -1 or 1
		end
	end
end)

local handBlockPitch, handBlockYaw = 0, 0
local hruxisSide = 1
function YawAntiAim(cmd)
	local mode = settings.HvH.AA
	local yaw, pitch = 0

	local invert = settings.HvH.Invert and keybinds.HvH.InvertKey

	if mode == "Jitter" then
		yaw = mRand(-180, 180)
	elseif mode == "Spin" then
		yaw = mNormalizeAng(CurTime() * 360)
	elseif mode == "180" then
		local delta = invert and -89 or 89
		yaw = GetPlayerYawInv() - (bSendPacket and delta or -delta)
	elseif mode == "Low delta rl" then
		local delta = invert and -74.9 or 74.9
		yaw = GetPlayerYawInv() - (bSendPacket and 0 or delta)
	elseif mode == "Low delta fk" then
		local delta = invert and -74.9 or 74.9
		yaw = GetPlayerYawInv() - (bSendPacket and delta or 0)
	elseif mode == "Legit" then
		local angs = settings.Aimbot.Silent and silentangles or cmd:GetViewAngles()
		pitch = angs.p

		if bSendPacket then
			yaw = angs.y
		else
			local delta = invert and -44 or 44
			yaw = angs.y + delta
		end
	elseif mode == "LBY" then
		local angs = settings.Aimbot.Silent and silentangles or cmd:GetViewAngles()
		yaw = samoware.GetCurrentLBY(me:EntIndex()) + (bSendPacket and 180 or 0)
	elseif mode == "LBY Break" or mode == "LBY Break legit" then
		if mode == "LBY Break" then
			yaw = GetPlayerYawInv()
		else
			yaw = settings.Aimbot.Silent and silentangles.y or cmd:GetViewAngles().y
		end

		if me:GetVelocity():Length2D() > 1 then
			-- Use LBY aa
			yaw = samoware.GetCurrentLBY(me:EntIndex()) + (bSendPacket and 180 or 0)
		else
			-- Use LBY Break aa

			local side = invert and -1 or 1

			local minDelta = settings.HvH["LBY Min delta"]
			local breakDelta = settings.HvH["LBY Break delta"]

			if !bSendPacket then
				local lbyTarget = samoware.GetTargetLBY(me:EntIndex())
				if mabs(mNormalizeAng(lbyTarget - fakeangles.y)) < minDelta then
					-- Set LBY to opposite side
					yaw = mNormalizeAng(fakeangles.y + breakDelta * side)
				else
					local lbyCurrent = samoware.GetCurrentLBY(me:EntIndex())
					yaw = mNormalizeAng(lbyCurrent - 44 * side)
				end
			end
		end
	elseif mode == "Anti BruteForce" then
		if IsValid(aaply) and aaply.sw_anti_bruteforce_angle then
			local delta = invert and 89 or -89
			yaw = GetPlayerYawInv() + (bSendPacket and delta or -delta) * aaply.sw_anti_bruteforce_angle
		else
			yaw = silentangles.y
		end
	elseif mode == "HandBlock" and aaply then
		local headId = FindBone(me, "Head")
		local headPos = me:GetBoneMatrix(headId):GetTranslation()

		local handId = FindBone(me, "ValveBiped.Bip01_R_Hand")
		local handPos = me:GetBoneMatrix(handId):GetTranslation()

		local eyePos = aaply:EyePos()

		-- Calculate pitch
		local SMOOTH_FACTOR = 1 / 1.6
		do
			local diff = handPos - eyePos
			local pitchDiff = math.deg(math.asin(diff.z / diff:Length()))

			handBlockPitch = mClamp(mNormalizeAng(handBlockPitch + pitchDiff * SMOOTH_FACTOR), -90, 90)
			pitch = handBlockPitch
		end

		if handBlockPitch < -60 or handBlockPitch > 60 then
			pitch = 89
			yaw = GetPlayerYawInv()
		else
			-- Calculate yaw
			do
				local vecA = handPos - headPos
				vecA.z = 0

				local vecB = handPos - eyePos
				vecB.z = 0

				local product = vecA:Dot(vecB)
				local length = vecA:Length2D() * vecB:Length2D()

				local yawDiff = 180 - mdeg(math.acos(mClamp(product / length, -1, 1)))
				local sign = vecA:Cross(vecB).z > 0
				if sign then
					yawDiff = -yawDiff
				end

				handBlockYaw = mNormalizeAng(handBlockYaw + yawDiff * SMOOTH_FACTOR)
				yaw = handBlockYaw
			end
		end
	elseif mode == "Hruxis" then
		if bSendPacket then
			hruxisSide = hruxisSide * -1
		end

		-- 360/s
		local swaySpeed = (ticksUntilSend + 1) / 12 * math.pi
		local swayAmount = math.sin(CurTime() * swaySpeed) * 45

		yaw = GetPlayerYawInv() + 55 * hruxisSide + swayAmount * hruxisSide * -1

		-- Break lby
		/*if !bSendPacket and chokes == 1 then --ticksUntilSend < 2 then
			print(bSendPacket)
			local lby = samoware.GetCurrentLBY(me:EntIndex())
			yaw = mNormalizeAng(lby - 180)
		end*/
		--cmd:SetSideMove(cmd:CommandNumber() % 2 == 0 and -25 or 25)
	end

	return yaw, pitch
end

end

local PitchAntiAim

do

local pitchflip = false

function PitchAntiAim(cmd)
	local mode = settings.HvH.FakePitch
	local pitch = 0

	if bSendPacket then
		pitchflip = !pitchflip
	end

	if mode == "Custom" then
		pitch = settings.HvH["Real Pitch"]
	elseif mode == "Fake down" then
		pitch = -180
	elseif mode == "Fake fake down" then
		pitch = 180.00000762939
	elseif mode == "Poseparam break jitter" then
		pitch = pitchflip and -180 or 271
	elseif mode == "Poseparam break fake down" then
		pitch = pitchflip and 179.9 or 89
	elseif mode == "Jitter" then
		pitch = mRand(-89, 89)
	end

	return pitch
end

end

local Crimwalk

do

local crimwalkcombos = {

}

local holdtypes = {
	low = {
		{"ar2", 4}, {"smg", 4}, {"shotgun", 3}, {"crossbow", 3},
		{"pistol", 5}, {"revolver", 4}, {"physgun", 2}, {"melee", 3},
		{"grenade", 4}, {"rpg", 1}, {"camera", 4}
	},
	high = {
		{"ar2", 3}, {"smg", 3}, {"shotgun", 3}, {"crossbow", 3},
		{"pistol", 4}, {"revolver", 5}, {"physgun", 2}, {"melee", 5},
		{"grenade", 5}, {"rpg", 5}, {"camera", 4}
	}
}

local angles = {
	{89, -89},
	{180.00000762939, -180},
	{89, -180},
	{180.00000762939, 180.00008},
	{89, 180.00008}
}

local function AddCombo(q, a1, a2, h1, h2)
	crimwalkcombos[#crimwalkcombos + 1] = {
		q = q,
		a1 = a1, a2 = a2,
		h1 = h1, h2 = h2
	}
end


for i = 1, #holdtypes.low do
	local hl = holdtypes.low[i]
	for j = 1, #holdtypes.high do
		local hj = holdtypes.high[j]
		for k = 1, #angles do
			local a = angles[k]
			AddCombo(hl[2] + hj[2], a[1], a[2], hl[1], hj[1])
		end
	end
end

local function GetCrimwalk()
	local weaps = me:GetWeapons()
	local holdtypes = {}

	for i = 1, #weaps do
		local weap = weaps[i]
		local weapht = weap:GetHoldType()

		if !holdtypes[weapht] then
			holdtypes[weapht] = {weap}
			continue
		end

		local ht = holdtypes[weapht]
		ht[#ht + 1] = weap
	end

	local available = {}

	for i = 1, #crimwalkcombos do
		local v = crimwalkcombos[i]
		local h1, h2 = v.h1, v.h2

		if !holdtypes[h1] or !holdtypes[h2] then
			continue
		end

		available[#available + 1] = v
	end

	local qmax = 0
	for i = 1, #available do
		local q = available[i].q
		if q > qmax then
			qmax = q
		end
	end

	local bestcrimwalks = {}
	local bestq = 0

	for i = 1, #available do
		local v = available[i]

		if v.q == qmax then
			local weaps1 = holdtypes[v.h1]
			local weaps2 = holdtypes[v.h1]

			v.w1 = weaps1[mrandom(#weaps1)]
			v.w2 = weaps2[mrandom(#weaps2)]

			bestcrimwalks[#bestcrimwalks + 1] = v
			bestq = v.q
		end
	end

	return bestcrimwalks
end

local ci, nextchange = 1, 0
function Crimwalk(cmd)
	local crimwalks = GetCrimwalk()

	if #crimwalks == 0 then return end

	if CurTime() > nextchange then
		ci = mrandom(1, #crimwalks)
		nextchange = CurTime() + 0.3
	end

	if ci > #crimwalks then
		ci = mrandom(1, #crimwalks)
	end

	local cw = crimwalks[ci]

	cmd:SelectWeapon(cmd:CommandNumber() % 2 == 0 and cw.w1 or cw.w2)
	cmd:SetViewAngles(Angle(cmd:CommandNumber() % 2 == 0 and cw.a1 or cw.a2, GetPlayerYawInv() - (mrandom(0, 1) == 0 and -45 or 45), 0))

	if cmd:CommandNumber() % 2 == 0 then
		cmd:SetButtons(bit.bor(cmd:GetButtons(), IN_DUCK))
	end

	if cmd:CommandNumber() % 4 == 0 then
		local vel = me:GetVelocity()
		local spd = vel:Length2D()
		local dir = vel:Angle()

		dir.y = cmd:GetViewAngles().y - dir.y

		local negDir = dir:Forward() * -spd

		cmd:SetForwardMove(negDir.x)
		cmd:SetSideMove(negDir.y)
	end
end

end

local function Freestand(cmd)
	if !IsValid(aaply) then return false end

	local headpos = me:GetBonePosition(me:LookupBone("ValveBiped.Bip01_Head1"))
	if !headpos then return end

	local selfpos = me:GetPos()
	local headoffset = Vector(selfpos.x, selfpos.y, headpos.z):Distance(headpos) + 5

	local found = true

	local pos = aaply:WorldToLocal(selfpos)
	local bearing = mdeg(-math.atan2(pos.y, pos.x)) + 180 + 90
	local left, right = bearing - 180 - 90, bearing - 180 + 90

	local function CheckYaw(yaw)
		yaw = mrad(yaw)
		local x, y = msin(yaw), mcos(yaw)

		local headoffsetvec = Vector(x, y, 0) * headoffset
		headoffsetvec.z = headpos.z - selfpos.z

		local tr = TraceLine({
			start = aaply:EyePos() + aaply:GetVelocity() * TICK_INTERVAL * 4,
			endpos = selfpos + headoffsetvec,
			filter = aaply
		})

		return tr.Fraction < 1 and tr.Entity != me
	end

	local function Normalize(ang) return 360 - ang + 90 end

	local leftcheck, rightcheck = CheckYaw(left), CheckYaw(right)

	left, right = Normalize(left), Normalize(right)

	do
		local headlocal = me:WorldToLocal(headpos)
		if headlocal.x > 0 then
			left, right = right, left
		end
	end

	if leftcheck and rightcheck then
		return false
	elseif leftcheck then
		return true, left , right
	elseif rightcheck then
		return true, right, left
	end

	return false
end

local function ShiftTickbase(amount)
	/*
	local maxshift = mmax(MAX_TICKBASE_SHIFT - ticksshiftedtotal - chokes, 0)

	amount = mClamp(amount or maxshift, 0, maxshift)

	shiftingtickbase = true
	for i = 1, amount do
		samoware.CL_Move()
	end
	shiftingtickbase = false

	ticksshifted = amount
	ticksshiftedtotal = ticksshiftedtotal + ticksshifted + chokes

	if ticksshiftedtotal >= MAX_TICKBASE_SHIFT then
		tickbaseshiftcharge = false
	end
	*/

	local ticksAllowed = samoware.GetTicksAllowed()
	local maxShift = mmax(ticksAllowed - chokes, 0)
	amount = mClamp(amount or maxShift, 0, maxShift)

	print("SHIFT", amount, ticksAllowed)
	samoware.SetTickbaseShift(amount)
end

local function RechargeShift()
	/*
	if ticksshiftedtotal == 0 then
		samoware.SetOutSequenceNr(samoware.GetOutSequenceNr() - MAX_TICKBASE_SHIFT)
	else
		samoware.SetOutSequenceNr(samoware.GetOutSequenceNr() - ticksshiftedtotal)
	end

	tickbaseshiftcharging = true
	ticksshifted = 0
	ticksshiftedtotal = 0
	*/

	local ticksToRecharge = MAX_TICKBASE_SHIFT - samoware.GetTicksAllowed()
	print("Charge", MAX_TICKBASE_SHIFT, samoware.GetTicksAllowed())
	samoware.SetTickbaseRecharge(ticksToRecharge)
end

local FakeEblan

do

local fakeeblan_factor = 3
if (1 / TICK_INTERVAL) >= 66 then
	fakeeblan_factor = 4
end

local nextm9k = 0
local prevack = 0
local numerrors = 0
function FakeEblan(cmd)
	if !bSendPacket then return end
	if keybinds.HvH.FakeEblanKey then
		if settings.HvH.FakeEblanMode == "Mega" then
			samoware.SetOutSequenceNr(samoware.GetOutSequenceNr() + mrandom(0, settings.HvH["FakeEblan strength"]))
		elseif settings.HvH.FakeEblanMode == "Slippery" then
			samoware.SetOutSequenceNr(samoware.GetOutSequenceNr() + fakeeblan_factor * settings.HvH["FakeEblan strength"] )
		elseif settings.HvH.FakeEblanMode == "Airstuck" then
			samoware.SetOutSequenceNr(samoware.GetOutSequenceNr() + 2500)
		elseif settings.HvH.FakeEblanMode == "M9K" then
			if CurTime() > nextm9k and band(cmd:GetButtons(), IN_ATTACK) != 0 then
				samoware.SetOutSequenceNr(samoware.GetOutSequenceNr() + 5000)
				nextm9k = CurTime() + 0.5
			end
		else
			samoware.SetOutSequenceNr(samoware.GetOutSequenceNr() + 5000 * settings.HvH["FakeEblan strength"])
		end
	end

	local curack = samoware.GetOutSequenceNrAck()
	local diff = curack - prevack
	if diff == 0 then
		numerrors = numerrors + 1
		if numerrors > 33 then
			print(("[samoware] Too much lost packets, fixing (delta %d)"):format(samoware.GetOutSequenceNr() - samoware.GetOutSequenceNrAck()))
			samoware.SetOutSequenceNr(curack)

			numerrors = 0
		end
	else
		numerrors = numerrors - 1
		if numerrors < 0 then
			numerrors = 0
		end
	end

	prevack = curack
end

end

/*

-16.000000 -16.000000 0.000000	16.000000 16.000000 36.000000
-16.000000 -16.000000 0.000000	16.000000 16.000000 72.000000

*/

local prevpos = Vector()
local function DuckInAir(cmd)
	if me:IsOnGround() then return end

	local PRED_TICKS = 4

	local startPosUnducked = me:GetPos()
	local isDucking = band(me:GetFlags(), FL_DUCKING) != 0
	if isDucking then
		-- Fix player position
		startPosUnducked.z = startPosUnducked.z - (72 - 36)
	end

	local canDuck = false
	samoware.StartSimulation(me:EntIndex())
	for i = 1, PRED_TICKS do
		samoware.SimulateTick()

		local simData = samoware.GetSimulationData()

		local maxs = me:OBBMaxs()
		maxs.z = 72 -- Standing height

		if isDucking then
			-- Fix player position
			simData.m_vecAbsOrigin.z = simData.m_vecAbsOrigin.z - (72 - 36)
		end

		local trace = TraceHull({
			start = startPosUnducked,
			endpos = simData.m_vecAbsOrigin,
			mins = me:OBBMins(),
			maxs = maxs,
			filter = me,
			mask = MASK_PLAYERSOLID
		})

		canDuck = !me:IsOnGround() and !trace.Hit
	end
	samoware.FinishSimulation()

	if canDuck and mrandom(0, 1) == 1 then
		cmd:SetButtons(bor(cmd:GetButtons(), IN_DUCK))
	end
end

local function FakeDuck(cmd)
	if !me:IsOnGround() then return end

	local sendduck = bSendPacket
	if settings.HvH["Invert FakeDuck"] then
		sendduck = !sendduck
	end

	if cmd:KeyDown(IN_DUCK) then return end

	if sendduck then
		cmd:SetButtons(bor(cmd:GetButtons(), IN_DUCK))
	else
		cmd:RemoveKey(IN_DUCK)
	end
end

local function SetBreakLagcompTicks()
	local vel = me:GetVelocity()
	local speed = vel:Length2D()

	local dst_per_tick = speed * TICK_INTERVAL

	local chokes = math.ceil(64 / dst_per_tick)
	chokes = mClamp(chokes, 1, 21)

	settings.HvH["FakeLag choke"] = chokes
	-- if settings.HvH["Randomize choke"] then
	-- 	settings.HvH["FakeLag choke"] = settings.HvH["FakeLag choke"] + mrandom(0, 21 - chokes)
	-- end
end

local function ESP()
	local dist = settings.Visuals.ESPDistance
	dist = dist * dist

	local realtime = RealTime()
	local selfpos = me:GetPos()
	for k, v in ipairs(player.GetAll()) do
		if !v:Alive() or v == me then continue end

		local dormant = v:IsDormant()
		if settings.Visuals.Dormant and dormant then
			dormant = (realtime - v.sw_last_dormant) > 5
		end

		if dormant then continue end

		local vpos = v:GetPos()

		if vpos:DistToSqr(selfpos) > dist then continue end

		local vcenter = v:OBBCenter()
		local omin, omax = v:OBBMins(), v:OBBMaxs()
		local postbl = {
			(vpos + Vector(omin.x, omin.y, omax.z)):ToScreen(),
			(vpos + Vector(omin.x, omax.y, omax.z)):ToScreen(),
			(vpos + Vector(omax.x, omax.y, omax.z)):ToScreen(),
			(vpos + Vector(omax.x, omin.y, omax.z)):ToScreen(),
			(vpos + Vector(omin.x, omin.y, omin.z)):ToScreen(),
			(vpos + Vector(omin.x, omax.y, omin.z)):ToScreen(),
			(vpos + Vector(omax.x, omax.y, omin.z)):ToScreen(),
			(vpos + Vector(omax.x, omin.y, omin.z)):ToScreen()
		}

		local x1, x2, y1, y2 = math.huge, -math.huge, math.huge, -math.huge

		local visible = false
		for i = 1, #postbl do
			local v = postbl[i]
			if v.visible then visible = true end

			if v.x < x1 then x1 = v.x end
			if v.x > x2 then x2 = v.x end
			if v.y < y1 then y1 = v.y end
			if v.y > y2 then y2 = v.y end
		end

		if !visible then continue end

		local teamcolor = team.GetColor(v:Team())

		if settings.Visuals.Dormant and v:IsDormant() then
			teamcolor.a = (1 - ((realtime - v.sw_last_dormant) / 5)) * 255
		end

		if settings.Visuals.Box then
			surface_SetDrawColor(teamcolor)
			surface.DrawOutlinedRect(x1, y1, x2 - x1, y2 - y1)
		end

		local i = 0
		surface_SetTextColor(color_white)
		surface_SetFont("swcc_text_esp")

		local upcenter = x1 + (x2 - x1) * 0.5

		if settings.Visuals.Name then
			local name = v:Nick()

			local tw, th = surface.GetTextSize(name)

			surface_SetTextPos(upcenter - tw * 0.5, y1 - th - th * i)
			surface_DrawText(name)

			i = i + 1
		end

		if settings.Visuals.Rank then
			local rank = v:GetUserGroup()

			local tw, th = surface.GetTextSize(rank)

			surface_SetTextPos(upcenter - tw * 0.5, y1 - th - th * i)
			surface_DrawText(rank)

			i = i + 1
		end

		if settings.Visuals.Weapon then
			local weap = v:GetActiveWeapon()
			if IsValid(weap) then
				local class = weap:GetClass()
				local tw, th = surface.GetTextSize(class)

				surface_SetTextPos(upcenter - tw * 0.5, y1 - th - th * i)
				surface_DrawText(class)

				i = i + 1
			end
		end

		local health, armor = v:Health(), v:Armor()

		if settings.Visuals.Health then
			local str = tostring(health)
			if armor > 0 then
				str = str .. " [" .. armor .. "]"
			end

			local tw, th = surface.GetTextSize(str)

			surface_SetTextPos(upcenter - tw * 0.5, y1 - th - th * i)
			surface_DrawText(str)

			i = i + 1
		end

		if settings.Visuals.Skeletons then
			surface_SetDrawColor(color_white)
			for i = 0, v:GetBoneCount() - 1 do
				local parent = v:GetBoneParent(i)
				if !parent then continue end

				local bonepos = v:GetBonePosition(i)
				if bonepos == vpos then continue end

				local parentpos = v:GetBonePosition(parent)
				if !bonepos or !parentpos then continue end

				local p1, p2 = bonepos:ToScreen(), parentpos:ToScreen()

				surface_DrawLine(p1.x, p1.y, p2.x, p2.y)
			end
		end

		if settings.Visuals.HealthBar then
			local maxhealth = v:GetMaxHealth()

			local healthfrac = mmin(health / maxhealth, 1)
			local height = healthfrac * (y2 - y1)

			local c = healthfrac * 255
			surface_SetDrawColor(255 - c,  c, 0)
			surface_DrawRect(x1 - 3 - 2, y1, 3, height)
		end

		if settings.Visuals.Extras then
			local flags = {}

			if v:IsTyping() then
				flags[#flags + 1] = "Typing"
			end

			if v:IsPlayingTaunt() then
				flags[#flags + 1] = "Taunting"
			end

			for i = 0, 13 do
				if v:IsValidLayer(i) then
					if v:GetSequenceActivityName(v:GetLayerSequence(i)):find("RELOAD") then
						flags[#flags + 1] = "Reloading"
						break
					end
				end
			end

			if v.sw_prev_pos and v.sw_cur_pos and v.sw_prev_pos:DistToSqr(v.sw_cur_pos) > 4096 then
				flags[#flags + 1] = "Breaking LC"
			end

			for i = 1, #flags do
				local str = flags[i]
				local tw, th = surface.GetTextSize(str)

				surface_SetTextPos(x2, y1 - th + th * i)
				surface_DrawText(str)
			end
		end
	end
end

local AutoStrafe, BunnyHop

do

local autostrafe_wish_dir = 0
local autostrafe_calc_dir = 0
local autostrafe_rough_dir = 0
local autostrafe_transition = false
local autostrafe_step = 0

local function GetRoughDir(base, true_dir)
	-- Make array with our four rough directions
	local minimum = {0, 180, 135, -135, 90, -90, 45, -45}
	local best_angle, best_delta = 181, 999999999

	-- Loop through our rough directions and find which one is closest to our true direction
	for i = 1, #minimum do
		local rough_dir = base.y + minimum[i]
		local delta = mabs(mNormalizeAng(true_dir - rough_dir))

		-- Only the smallest delta wins out
		if delta < best_delta then
			best_angle = rough_dir
			best_delta = delta
		end
	end

	return best_angle
end

local function CalcForward(cmd)
	if mabs(cmd:GetForwardMove()) < 1 then
		if mabs(cmd:GetSideMove()) < 1 and mabs(cmd:GetMouseX()) < 1 and me:IsOnGround() then
			return 10000
		else
			local mx = mabs(cmd:GetMouseX()) * 2
			if mx == 0 then
				mx = 1
			elseif mx > 80 then
				return
			end

			local spd = me:GetVelocity():Length2D()
			if spd == 0 then
				spd = 1
			end

			return 50000 * (1 / (spd * mx))
		end
	end
end

local function CalcSide(cmd)
	if !me:IsOnGround() and mabs(cmd:GetSideMove()) < 1 then
		if mabs(cmd:GetMouseX()) > 1 then
			return cmd:GetMouseX() * 12
		else
			return cmd:CommandNumber() % 2 == 0 and 10000 or -10000
		end
	end
end

local function DefaultAutoStafe(cmd)
	if !cmd:KeyDown(IN_JUMP) then return end

	--[=[if !me:IsOnGround() then
		if mabs(cmd:GetMouseX()) < 1 then
			cmd:SetSideMove(cmd:GetMouseX() < 0 and -10000 or 10000)
		else
			local velocity = me:GetVelocity()
			cmd:SetForwardMove(36000 * 4 / velocity:Length2D())
			cmd:SetSideMove(((cmd:CommandNumber() % 2) == 0) and -10000 or 10000)
		end
	elseif me:IsOnGround() and cmd:KeyDown(IN_JUMP) then
		if mabs(cmd:GetMouseX()) < 1 then
			cmd:SetForwardMove(10000)
		end
	end
	--]=]


	local moveType = me:GetMoveType()
	if me:WaterLevel() > 1 or moveType == MOVETYPE_LADDER or moveType == MOVETYPE_NOCLIP then return end

	local function GetMaxAngleDelta(speed)
		local x = (30 - (10 * 300 * TICK_INTERVAL)) / speed
		if x > -1 and x < 1 then
			return math.acos(x) + 0.175
		end

		return 0
	end

	local velocity = me:GetVelocity()

	local velYaw = mdeg(math.atan2(velocity.y, velocity.x))
	local strafeSide = (mNormalizeAng(velYaw - cmd:GetViewAngles().y) > 0) and 1 or -1

	local maxMove = settings.Misc["SimplAC safe mode"] and 10000 or 5250

	local forwardMove = cmd:GetForwardMove()
	if mabs(forwardMove) < 1 and me:IsOnGround() and velocity:Length2D() < 500 then
		forwardMove = maxMove
	end

	local sideMove = cmd:GetSideMove()
	if mabs(sideMove) < 1 then
		if settings.Misc.AirStrafe then
			sideMove = maxMove * strafeSide
		elseif cmd:GetMouseX() != 0 then
			sideMove = (cmd:GetMouseX() > 0) and maxMove or -maxMove
		else
			sideMove = 0
		end

		cmd:RemoveKey(IN_MOVERIGHT)
		cmd:RemoveKey(IN_MOVELEFT)

		local move = (strafeSide > 0) and IN_MOVERIGHT or IN_MOVELEFT
		cmd:SetButtons(bor(cmd:GetButtons(), move))
	end

	if !settings.Misc["SimplAC safe mode"] then
		local view = cmd:GetViewAngles()
		view.y = view.y + GetMaxAngleDelta(velocity:Length2D()) * strafeSide

		cmd:SetViewAngles(view)
	end

	if me:IsOnGround() then
		cmd:SetButtons(bor(cmd:GetButtons(), IN_SPEED))
	end

	cmd:SetForwardMove(forwardMove)
	cmd:SetSideMove(sideMove)
	--]=]
end

function AutoStrafe(cmd)
	if !settings.Misc["Multidir. autostrafe"] then
		DefaultAutoStafe(cmd)
		return
	end

	if !cmd:KeyDown(IN_JUMP) then return end
	if me:GetMoveType() == MOVETYPE_NOCLIP or me:GetMoveType() == MOVETYPE_LADDER or me:WaterLevel() > 1 then return end

	local base = settings.Aimbot.Silent and silentangles or me:EyeAngles()
	if settings.Misc["Multidir. autostrafe"] then
		if me:IsOnGround() and !cmd:KeyDown(IN_JUMP) then
			autostrafe_wish_dir = base.y + 0
			autostrafe_transition = false
		else
			if cmd:KeyDown(IN_FORWARD) then
				if cmd:KeyDown(IN_MOVELEFT) then
					autostrafe_wish_dir = base.y + 45 -- forward left
				elseif cmd:KeyDown(IN_MOVERIGHT) then
					autostrafe_wish_dir = base.y + -45 -- forward right
				else
					autostrafe_wish_dir = base.y + 0 -- forward
				end
			elseif cmd:KeyDown(IN_BACK) then
				if cmd:KeyDown(IN_MOVELEFT) then
					autostrafe_wish_dir = base.y + 135 -- back left
				elseif cmd:KeyDown(IN_MOVERIGHT) then
					autostrafe_wish_dir = base.y + -135 -- back right
				else
					autostrafe_wish_dir = base.y + 180 -- back
				end
			elseif cmd:KeyDown(IN_MOVELEFT) then
				autostrafe_wish_dir = base.y + 90 -- left
			elseif cmd:KeyDown(IN_MOVERIGHT) then
				autostrafe_wish_dir = base.y + -90 -- right
			else
				autostrafe_wish_dir = base.y + 0

				-- fix antiaim strafing
				cmd:SetButtons(bor(cmd:GetButtons(), IN_FORWARD))
			end
		end
	else
		autostrafe_transition = false
	end

	cmd:SetButtons(bor(cmd:GetButtons(), IN_SPEED))

	local true_dir = me:GetVelocity():Angle().y
	local speed_rotation = mmin(mdeg(math.asin(30 / me:GetVelocity():Length2D())) * 0.5, 45)

	if settings.Misc["Multidir. autostrafe"] and autostrafe_transition then
		-- Calculate the step by using our ideal strafe rotation
		local ideal_step = speed_rotation + autostrafe_calc_dir
		autostrafe_step = mabs(mNormalizeAng(autostrafe_calc_dir - ideal_step))

		-- Check when the calculated direction arrives close to the wish direction
		if mabs(mNormalizeAng(autostrafe_wish_dir - autostrafe_calc_dir)) > autostrafe_step then
			local add = mNormalizeAng(autostrafe_calc_dir + autostrafe_step)
			local sub = mNormalizeAng(autostrafe_calc_dir - autostrafe_step)

			-- Step in direction that gets us closer to our wish direction
			if mabs(mNormalizeAng(autostrafe_wish_dir - add)) >= mabs(mNormalizeAng(autostrafe_wish_dir - sub)) then
				autostrafe_calc_dir = autostrafe_calc_dir - autostrafe_step
			else
				autostrafe_calc_dir = autostrafe_calc_dir + autostrafe_step
			end
		else
			autostrafe_transition = false
		end
	elseif settings.Misc["Multidir. autostrafe"] then
		-- Get rough direction and setup calculated direction only when not transitioning
		autostrafe_rough_dir = GetRoughDir(base, true_dir)
		autostrafe_calc_dir = autostrafe_rough_dir

		-- When we have a difference between our current (rough) direction and our wish direction, then transition
		if autostrafe_rough_dir != autostrafe_wish_dir then
			autostrafe_transition = true
		end
	else
		autostrafe_transition = false
	end

	if autostrafe_transition then
		cmd:SetForwardMove(0)
		cmd:SetSideMove(0)

		local movex = CalcForward(cmd) or 0
		local movey = CalcSide(cmd) or 0

		-- Calculate ideal rotation based on our newly calculated direction
		local direction = (cmd:CommandNumber() % 2 == 0 and speed_rotation or -speed_rotation) + autostrafe_calc_dir

		-- Rotate our direction based on our new, defininite direction
		local rotation = mrad(base.y - direction)

		local cos_rot = mcos(rotation)
		local sin_rot = msin(rotation)

		local forwardmove = (cos_rot * movex) - (sin_rot * movey)
		local sidemove = (sin_rot * movex) + (cos_rot * movey)

		-- Apply newly rotated movement
		cmd:SetForwardMove(forwardmove)
		cmd:SetSideMove(sidemove)
	else
		-- fix antiaim strafing
		cmd:SetButtons(bor(cmd:GetButtons(), IN_FORWARD))

		local fwd, side = CalcForward(cmd), CalcSide(cmd)
		if fwd then
			cmd:SetForwardMove(fwd)
		end

		if side then
			cmd:SetSideMove(side)
		end
	end
end

local cw, ch = SCRW / 2, SCRH / 2
local indicator_cur = Color(255, 45, 45)
local indicator_targ = Color(45, 255, 45)
local scale = cw * 0.1
AddHook("HUDPaint", function()
	if settings.Misc["Multidir. indicator"] then
		local cur = mrad(mNormalizeAng(me:GetVelocity():Angle().y))
		local xcur, ycur = msin(cur) * scale, mcos(cur) * scale

		surface_SetDrawColor(indicator_cur)
		surface_DrawLine(cw, ch, cw + xcur, ch + ycur)

		local targ = mrad(mNormalizeAng(autostrafe_calc_dir))
		local xtarg, ytarg = msin(targ) * scale, mcos(targ) * scale

		surface_SetDrawColor(indicator_targ)
		surface_DrawLine(cw, ch, cw + xtarg, ch + ytarg)
	end

	draw.SimpleText(me:GetVelocity():Length(), "DermaLarge", 128, 128)
end)

local wasOnGround = false
function BunnyHop(cmd)
	if !cmd:KeyDown(IN_JUMP) then return end

	if !me:IsOnGround() or wasOnGround then
		wasOnGround = false
		cmd:RemoveKey(IN_JUMP)
	else
		wasOnGround = true
	end
end

end

local jesusVel = me:GetVelocity()
local function MichaelJackson(cmd)
	local function getDecelRatio(velocity)
		local sv_friction = GetConVar("sv_friction"):GetFloat()
		local sv_stopspeed = GetConVar("sv_stopspeed"):GetFloat()

		do
			local velAngle = velocity:Angle()
			velAngle.y = velAngle.y - cmd:GetViewAngles().y

			local velDir = velAngle:Forward()
			velDir = velDir * 10000
			velDir.z = 0

			local fRatio = me:GetMaxSpeed() / math.sqrt(velDir:Length())
			velDir.x = velDir.x * fRatio
			velDir.y = velDir.y * fRatio
			velDir.z = 0

			velocity = velocity - velDir
		end

		local friction = sv_friction * 1
		local speed = velocity:Length()
		local control = mmax(speed, sv_stopspeed)
		local drop = control * friction * TICK_INTERVAL
		local newSpeed = mmax(speed - drop, 0)
		local ratio = newSpeed / speed

		return ratio
	end

	local desiredVelocity = 50

	local ticksToStop = 0
	local predictedVelocity = jesusVel --me:GetVelocity()
	while ticksToStop < 21 and predictedVelocity:Length() > desiredVelocity do
		local ratio = getDecelRatio(predictedVelocity)
		predictedVelocity = predictedVelocity * ratio
		ticksToStop = ticksToStop + 1
	end

	print(ticksToStop)

	local shouldFastStop = chokes < 1 or chokes > (21 - ticksToStop)
	if shouldFastStop then
		local velocity = me:GetVelocity()
		local velAngle = velocity:Angle()
		velAngle.y = velAngle.y - cmd:GetViewAngles().y

		local velDir = velAngle:Forward()
		velDir = (velDir + Vector(500, 0, 0)) * 10000 --(velocity:Length2D() + 0) --(cmd:KeyDown(IN_SPEED) and 250 or 100))

		cmd:SetForwardMove(-velDir.x)
		cmd:SetSideMove(velDir.y)
	else
		jesusVel = me:GetVelocity()
	end
end

local StartCircleStrafe, FinishCircleStrafe
do
	local function InitializeMoveData(ply, cmd, out)
		local moveData = out or {}

		-- Cached C calls
		moveData.gravity = ply:GetGravity()
		moveData.currentGravity = -physenv.GetGravity().z
		moveData.jumpPower = ply:GetJumpPower()
		moveData.groundEntity = ply:GetGroundEntity()
		moveData.obbMins = ply:OBBMins()
		moveData.obbMaxs = ply:OBBMaxs()
		moveData.m_surfaceFriction = ply:GetFriction()

		-- ConVars
		moveData.sv_airaccelerate = GetConVar("sv_airaccelerate"):GetFloat()

		moveData.m_vecViewAngles = cmd:GetViewAngles()
		moveData.m_vecVelocity = ply:GetVelocity()
		moveData.m_vecAbsOrigin = ply:GetPos()

		-- From simulation.cpp
		-- Set in threadedsimulation.cpp
		/*
		do
			local forward, right, up = moveData.m_vecViewAngles:Forward(), moveData.m_vecViewAngles:Right(), moveData.m_vecViewAngles:Up()
			moveData.m_flForwardMove = (moveData.m_vecVelocity.y - right.y / right.x * moveData.m_vecVelocity.x) / (forward.y - right.y / right.x * forward.x)
			moveData.m_flSideMove = (moveData.m_vecVelocity.x - forward.x * moveData.m_flForwardMove) / right.x
		end
		*/

		moveData.m_nButtons = cmd:GetButtons()
		moveData.m_nOldButtons = 0 -- TODO

		moveData.m_flMaxSpeed = ply:GetMaxSpeed()

		return moveData
	end

	local function GetIdealAngleDelta(speed, maxSpeed, airAccelerate)
		local term = (30 - (airAccelerate * maxSpeed * TICK_INTERVAL)) / speed
		if term > -1 and term < 1 then
			return mdeg(math.acos(term))
		end

		return 360
	end

	local function SolveCircle(a, b, c)
		local x12 = a.x - b.x
		local x13 = a.x - c.x

		local y12 = a.y - b.y
		local y13 = a.y - c.y

		local y31 = c.y - a.y
		local y21 = b.y - a.y

		local x31 = c.x - a.x
		local x21 = b.x - a.x

		local sx13 = a.x * a.x - c.x * c.x
		local sy13 = a.y * a.y - c.y * c.y

		local sx21 = b.x * b.x - a.x * a.x
		local sy21 = b.y * b.y - a.y * a.y

		local f = ((sx13) * (x12)
					+ (sy13) * (x12)
					+ (sx21) * (x13)
					+ (sy21) * (x13))
					/ (2 * ((y31) * (x12) - (y21) * (x13)))

		local g = ((sx13) * (y12)
					+ (sy13) * (y12)
					+ (sx21) * (y13)
					+ (sy21) * (y13))
					/ (2 * ((x31) * (y12) - (x21) * (y13)))


		local c = -(a.x * a.x) - (a.y * a.y) - 2 * g * a.x - 2 * f * a.y

		local h = -g
		local k = -f
		local sqrOfR = h * h + k * k - c

		return h, k, msqrt(sqrOfR)
	end

	local function GetPossiblePaths(count, precision, pos, velocity, velocityYaw, speed2d)
		local speed2dSafe = speed2d
		if speed2dSafe < 1e-4 then
			-- Prevent NaN in SolveCircle
			speed2dSafe = 1
		end

		local traces = {}
		for yawOffset = -90 - 45, 90 + 45, precision do
			local yaw = math.NormalizeAngle(velocityYaw + yawOffset)
			local yawDirection = Vector(math.cos(math.rad(yaw)), math.sin(math.rad(yaw)), 0)
			local trace = TraceHull({
				start = pos,
				endpos = pos + yawDirection * speed2dSafe * 2,
				mins = me:OBBMins(),
				maxs = me:OBBMaxs(),
				filter = me,
				mask = MASK_PLAYERSOLID
			})

			local distance = trace.StartPos:Distance(trace.HitPos)

			-- debugoverlay.Line(trace.StartPos, trace.HitPos, TICK_INTERVAL * 2, color_white, true)
			-- debugoverlay.Cross(trace.StartPos + yawDirection * distance * 0.5, 2, TICK_INTERVAL * 2, color_white, true)

			traces[#traces + 1] = {
				hitPos = trace.HitPos,
				distance = distance,
				yawOffset = yawOffset
			}
		end

		-- Sort by distance
		table.sort(traces, function(a, b)
			return a.distance > b.distance
		end)

		local results = {}
		for i = 1, math.min(count, #traces) do
			local trace = traces[i]
			results[i] = trace

			do
				local yaw = math.NormalizeAngle(velocityYaw + trace.yawOffset)
				local yawDirection = Vector(math.cos(math.rad(yaw)), math.sin(math.rad(yaw)), 0)
				debugoverlay.Cross(pos + yawDirection * trace.distance * 0.5, 5, TICK_INTERVAL * 2, Color(255, 0, 0), true)
			end
		end

		return results
	end

	local function CalculateAngularVelocities(possiblePaths, circleStrafeYaw, pos, velocity, velocityYaw, speed2d)
		local wishDir = Vector(mcos(mrad(circleStrafeYaw)), msin(mrad(circleStrafeYaw)), 0)

		local circleA = pos

		local angVelocities = {}
		for i = 1, #possiblePaths do
			angVelocities[i] = math.Remap(i, 1, #possiblePaths, -5, 5)
		end

		/*for i = 1, #possiblePaths do
			local path = possiblePaths[i]

			-- Solve for circle radius and center
			local circleB = pos + wishDir * speed2d * TICK_INTERVAL
			local circleC = path.hitPos

			local x, y, r = SolveCircle(circleA, circleB, circleC)
			if r != r then -- NaN
				circleB = pos + velocity * TICK_INTERVAL
				x, y, r = SolveCircle(circleA, circleB, circleC)
				if r != r then -- NaN again...
					circleB = pos + wishDir
					x, y, r = SolveCircle(circleA, circleB, circleC)
				end
			end

			-- Get angular velocity
			local circumference = math.pi * r * 2
			local period = circumference / (speed2d + 1e-6)
			local angularVelocity = mdeg((2 * math.pi) / period) * TICK_INTERVAL

			-- Fix velocity sign
			if path.yawOffset < 0 then
				angularVelocity = -angularVelocity
			end

			angVelocities[i] = angularVelocity
		end*/

		return angVelocities
	end

	local moveDataCache = {}
	local function CreateMovementData(angularVelocities, simTime, speed2d, cmd, circleStrafeYaw)
		local m_vecViewAngles = Angle(0, circleStrafeYaw, 0)
		for i = 1, #angularVelocities do
			local angularVelocity = angularVelocities[i]

			local moveData = InitializeMoveData(me, cmd, moveDataCache[i])
			moveData.timeUntilTimeout = simTime
			moveData.deltaTime = TICK_INTERVAL
			moveData.idealYawDelta = angularVelocity
			moveData.maxSpeedLoss = speed2d * 0.5
			moveData.m_vecViewAngles = m_vecViewAngles

			moveDataCache[i] = moveData
		end

		return moveDataCache
	end

	-- Bugged zones go here
	local MAP_AVOIDED_ZONES = {
		["rp_bangclaw"] = {
			-- Water shit
			{mins = Vector(-1011.354492, -2315.062500, -379.469147), maxs = Vector(-328.926392, -1405.410767, 1618.861328)},

			-- Subway entrance
			{mins = Vector(2862.246094, -3892.602539, -205.682663), maxs = Vector(3365.378662, -3623.068359, 466.894409)},

			-- Desert short corridor thing
			{mins = Vector(10648.275391, -4314.236328, 60), maxs = Vector(10752.227539, -2346.496826, 1063.019043)}
		}
	}

	local avoidedZones = MAP_AVOIDED_ZONES[game.GetMap()]
	local function IsInZone(zone, pos)
		local mins, maxs = zone.mins, zone.maxs
		local xIn = pos.x > mins.x and pos.x < maxs.x
		local yIn = pos.y > mins.y and pos.y < maxs.y
		local zIn = pos.z > mins.z and pos.z < maxs.z
		return xIn and yIn and zIn
	end

	local function ChooseBestAngularVelocity(angularVelocities, moveDatas, simTime, pos, speed2d)
		local bestAngularVelocityValid = false
		local bestAngularVelocity = nil
		local bestAngularVelocityDeltaSpeed = -math.huge
		local bestAngularVelocityDuration = 0
		local bestAngularVelocityTicks = nil

		samoware.StartThreadedSimulation(moveDatas)

		-- Wait for aimbot
		coroutine.yield()

		local results = samoware.FinishThreadedSimulation(#moveDatas)

		do
			hook.Add("PostDrawTranslucentRenderables", "dbg", function()
				surface.SetDrawColor(color_white)
				for i = 1, #results do
					local ticks = results[i]
					for j = 1, #ticks - 1 do
						local curTick, nextTick = ticks[j], ticks[j + 1]
						--debugoverlay.Line(curTick.m_vecAbsOrigin, nextTick.m_vecAbsOrigin, TICK_INTERVAL * 2, color_white, true)
						render.DrawLine(curTick.m_vecAbsOrigin, nextTick.m_vecAbsOrigin, color_white, false)
					end
				end
			end)
		end

		for i = 1, #results do
			local angularVelocity = angularVelocities[i]

			local ticks = results[i]
			local lastTick = ticks[#ticks]

			local isValid = lastTick.timeUntilTimeout > 0
			local deltaSpeed = lastTick.m_vecVelocity:Length2D() - speed2d

			-- Check if simulation ended up in avoided zone
			if avoidedZones and isValid then
				local endPos = lastTick.m_vecAbsOrigin
				for i = 1, #avoidedZones do
					isValid = isValid and !IsInZone(avoidedZones[i], endPos)
				end
			end

			-- isValid = isValid and lastTick.m_vecAbsOrigin.z > (400)


			if !bestAngularVelocity then
				bestAngularVelocityValid = isValid
				bestAngularVelocity = angularVelocity
				bestAngularVelocityDeltaSpeed = deltaSpeed
				bestAngularVelocityDuration = simTime - lastTick.timeUntilTimeout
				bestAngularVelocityTicks = ticks
			elseif isValid then
				if deltaSpeed > bestAngularVelocityDeltaSpeed then
					bestAngularVelocityValid = true
					bestAngularVelocity = angularVelocity
					bestAngularVelocityDeltaSpeed = deltaSpeed
					bestAngularVelocityDuration = simTime - lastTick.timeUntilTimeout
					bestAngularVelocityTicks = ticks
				end
			else
				if !bestAngularVelocityValid and deltaSpeed > bestAngularVelocityDeltaSpeed then
					bestAngularVelocityValid = false
					bestAngularVelocity = angularVelocity
					bestAngularVelocityDeltaSpeed = deltaSpeed
					bestAngularVelocityDuration = simTime - lastTick.timeUntilTimeout
					bestAngularVelocityTicks = ticks
				end
			end
		end

		return bestAngularVelocityValid,
				bestAngularVelocity,
				bestAngularVelocityDeltaSpeed,
				bestAngularVelocityDuration,
				bestAngularVelocityTicks
	end

	local bestAngularVelocityValid = false
	local bestAngularVelocity = nil
	local bestAngularVelocityDeltaSpeed = -math.huge
	local bestAngularVelocityDuration = 0
	local bestAngularVelocityDurationTicks = nil
	local bestAngularVelocityDurationTickCount = 0
	local circleStrafeYaw = nil
	local function CircleStrafe(cmd)
		local numJobs = settings.Misc["CStrafe jobs"]
		local simTime = settings.Misc["CStrafe sim time"]

		local pos = me:GetPos()
		local velocity = me:GetVelocity()
		local velocityYaw = math.deg(math.atan2(velocity.y, velocity.x))
		local speed, speed2d = velocity:Length(), velocity:Length2D()

		if !circleStrafeYaw then
			circleStrafeYaw = math.deg(math.atan2(velocity.y, velocity.x))
		end

		local possiblePaths = GetPossiblePaths(numJobs, 5, pos, velocity, velocityYaw, speed2d)
		local angularVelocities = CalculateAngularVelocities(possiblePaths, circleStrafeYaw, pos, velocity, velocityYaw, speed2d)
		local moveDatas = CreateMovementData(angularVelocities, simTime, speed2d, cmd, circleStrafeYaw)
		local isValid, angularVelocity, deltaSpeed, duration, ticks = ChooseBestAngularVelocity(angularVelocities, moveDatas, simTime, pos, speed2d)

		local notNil = angularVelocity != nil -- TODO: FIX THIS
		local justInitialized = !bestAngularVelocity
		local valid = !bestAngularVelocityValid and isValid
		local betterBySpeed = (deltaSpeed - bestAngularVelocityDeltaSpeed) > -5
		local longer = duration > bestAngularVelocityDuration
		local expired = bestAngularVelocityDuration <= 0
		local simulationError = false
		do
			local tickRel = engine.TickCount() - bestAngularVelocityDurationTickCount
			if bestAngularVelocityDurationTicks and tickRel > 0 then
				local curTick = bestAngularVelocityDurationTicks[tickRel]
				if curTick then -- Not expired?
					local posDiff = pos:Distance(curTick.m_vecAbsOrigin)
					simulationError = posDiff > 5
				end
			end
		end

		if notNil and (justInitialized or valid or (isValid and longer and betterBySpeed) or expired or simulationError) then
			do
				local causes = {}
				if justInitialized then							causes[#causes + 1] = "INIT" end
				if valid then									causes[#causes + 1] = "VALID" end
				if (isValid and longer and betterBySpeed) then	causes[#causes + 1] = "DUR&SPD" end
				if expired then									causes[#causes + 1] = "EXPIRED" end
				if simulationError then							causes[#causes + 1] = "SIMERR" end

				print("New path:", ("%.1f %s"):format(angularVelocity, table.concat(causes, ",")))
			end

			bestAngularVelocityValid = isValid
			bestAngularVelocity = angularVelocity
			bestAngularVelocityDeltaSpeed = deltaSpeed
			bestAngularVelocityDuration = duration
			bestAngularVelocityDurationTicks = ticks
			bestAngularVelocityDurationTickCount = engine.TickCount()
		end

		if !bestAngularVelocity then
			print("NO bestAngularVelocity!!!!!!!!!!!!!!")

			-- Restore initial state
			bestAngularVelocityValid = false
			bestAngularVelocity = 0
			bestAngularVelocityDeltaSpeed = -math.huge
			bestAngularVelocityDuration = 0
			bestAngularVelocityDurationTicks = 0
			bestAngularVelocityDurationTickCount = 0
		end

		-- Decrease current angular velocity duration
		bestAngularVelocityDuration = bestAngularVelocityDuration - TICK_INTERVAL
		circleStrafeYaw = math.NormalizeAngle(circleStrafeYaw + bestAngularVelocity)

		/*
		local sideMove
		if math.abs(bestAngularVelocity) < 1e-3 then
			sideMove = 0
		else
			sideMove = bestAngularVelocity > 0 and -5250 or 5250
		end

		cmd:AddKey(IN_SPEED)
		cmd:SetSideMove(sideMove)
		FixMove(cmd, Angle(0, circleStrafeYaw, 0))
		*/
	end

	local circleStrafeCoro = coroutine.create(function(cmd)
		while true do
			CircleStrafe(cmd)
			coroutine.yield()
		end
	end)

	local function AwaitThread(cmd)
		local succ, err = coroutine.resume(circleStrafeCoro, cmd)
		if !succ then
			error(debug.traceback(circleStrafeCoro, err))
		end
	end

	function StartCircleStrafe(cmd)
		if !settings.Misc.CStrafe or !keybinds.Misc.CStrafeKey then
			circleStrafeYaw = nil
			return
		end

		-- Wake the loop yield
		AwaitThread(cmd)
	end

	function FinishCircleStrafe(cmd)
		if !settings.Misc.CStrafe or !keybinds.Misc.CStrafeKey then
			circleStrafeYaw = nil -- Recalculate velocity yaw
			bestAngularVelocity = nil -- Initialize new angular velocity
			return
		end

		-- Wake ChooseBestAngularVelocity yield
		AwaitThread(cmd)

		if !bestAngularVelocity then
			return
		end

		local sideMove
		if math.abs(bestAngularVelocity) < 1e-3 then
			sideMove = 0
		else
			sideMove = bestAngularVelocity > 0 and -5250 or 5250
		end

		cmd:AddKey(IN_SPEED)
		cmd:SetForwardMove(0)
		cmd:SetSideMove(sideMove)
		FixMove(cmd, Angle(0, circleStrafeYaw, 0))
	end

	AddHook("PostDrawTranslucentRenderables", function(bDrawingDepth, bDrawingSkybox, isDraw3DSkybox)
		if bDrawingSkybox or isDraw3DSkybox then return end

		render.SetColorMaterial()

		-- Draw simulated ticks
		if bestAngularVelocityDurationTicks then
			local colorGood = Color(0, 255, 0)
			local colorBad = Color(255, 0, 0)
			local pathColor = bestAngularVelocityValid and colorGood or colorBad

			for i = 1, #bestAngularVelocityDurationTicks - 1 do
				local curTick = bestAngularVelocityDurationTicks[i]
				local nextTick = bestAngularVelocityDurationTicks[i + 1]
				render.DrawBeam(curTick.m_vecAbsOrigin, nextTick.m_vecAbsOrigin, 2, 0, 1, pathColor)
			end
		end

		-- Draw avoided zones
		if avoidedZones then
			cam.IgnoreZ(true)

			for i = 1, #avoidedZones do
				local zone = avoidedZones[i]
				local mins, maxs = zone.mins, zone.maxs
				local origin = mins + (maxs - mins) * 0.5
				local minsLocal = maxs - origin
				local maxsLocal = mins - origin

				local distance = me:GetPos():Distance(origin)
				local alpha = math.Clamp((2000 - distance) / 2000 * 125, 0, 125)

				local color = Color(215, 0, 0, alpha)
				render.DrawBox(origin, angle_zero, minsLocal, maxsLocal, color)
			end

			cam.IgnoreZ(false)
		end
	end)
end

local killrow_messages = {
	["Russian"] = {
		"%s lox)",
		"%s чет изи",
		"%s ого ты лох",
		"%s loshara",
		"%s гг изи",
		"%s слkбый ты слабый)",
		"%s ого соснул",
		"%s ахах хдд соснул пздц",
		"лоол рили %s)",
		"%s да ты /\\()X",
		"%s страшна вырубай)",
		"ну че ты %s",
		"кто соснул? %s соснул",
		"КАК МАТЬ???? ЖИВА???? %s",
		"%s играть научись",
		"таких лохов как %s я в жизни не встречал....",
		"%s ты медленый",
		"%s слепошара",
		"%s ты говно",
		"%s пиздец лох",
		"%s ну что",
		"че по ебалу?)) %s",
		"эхх лошок"
	},
	["English"] = {
		"eat shit",
		"eat a fat steaming cock you unpriviledged homosexual",
		"suck my universe sized dick",
		"drink my piss fucking faggot",
		"hop off my dick fucking nigger",
		"%s is so shit",
		"can you stop dying, %s?",
		"hey,".."%s? it's okay,try again next time!",
		"what the fuck was that %s?",
		"plan your next try in the respawn room!",
		"rekt",
		"owned",
		"lol",
		"you're a retard, %s",
		"there you go,back to the respawn",
		"you're bad, %s",
		"noob down",
		"lmao",
		"%s has died more times than native americans did back in the 1800's",
		"i bet you're insecure about your aim",
		"ahahah",
		"excuse me, %s, you have won the world record of the worst KD in history!",
		"there he goes back to the respawn room",
		"don't let the door hit you on the way out, %s!",
		"noob",
		"%s is a noob",
		"nerd",
		"pff",
		"ha",
		"ez",
		"%s is a nerd",
		"good job!",
		"try not to die next time, %s!",
	},
	["Opezdal"] = {

	},
	["Unizhenie"] = {

	},
	["Russian HvH"] = {
		"хуевый ресолвер",
		"хуевые фейклаги",
		"хуевый антиаим",
		"хуевый спинбот",
		"хуевый бхоп",
		"хуевый аим",
		"найс паста аимвара",
		"найс паста мемевара",
		"неужели это идиотбокс???",
		"ого идиотбокс???",
		"неужели это аосхак???",
		"ого аосхак???",
		"неужели это ехек хак???",
		"ого ехек хак???",
		"что за ебанутый у тебя чит?",
		"ez",
		"ezz",
		"изи",
		"ииииииизи",
		"упал",
		"спи",
		"отдыхай",
		"отлетел дебил)",
		"упал пастер",
		"пастер лег",
		"изи даун",
		"ору отлетела дура",
		"найс ресолвер стен",
		"найс ресолвер деревьев",
		"бро имажин ресолвинг ин гмод",
		"улетел фанат артемкинга4",
		"упал фанат артемкинга4",
		"ты куда стреляешь)))",
		"упал ннчик без самоваре",
		"умер ннчик без самоваре",
		"отдыхай ннчик без самоваре",
		"упал подписчик урбанички",
		"умер подписчик урбанички",
		"отдыхай подписчик урбанички",
		"енжинпред где???",
		"антиаим где???",
		"фейклаги где???",
		"антиаим не спас",
		"фейклаги не спасли",
		"даун с пастой отлетел",
		"упал баимер ебаный",
		"отлетел ебаный баимер))",
		"охуеть даун с пастой аимвара",
		"упал дебил",
		"выйди не позорься",
		"найс брейн иссуе",
		"найс кфг иссуе",
		"сука не позорься и ливни лол",
		"*DEAD* пофикси нищ",
		"нищий улетел",
		"набутылирован лол",
		"ебать ты красиво на бутылку упал",
		"хуя тебя опустили))",
		"прости что без смазки)",
		"обоссан",
		"обоссал юзера пасты аимвара",
		"алло это скорая? тут такая ситуация нищ упал)))",
		"на завод иди",
		"ебать тебя унесло",
		"ой нищий упал щас скорую вызовем",
		"научи потом как так сосать на хвх",
		"нихуя ты там как самолет отлетел",

		-- Vovse ne sp1zd1l
		"Найс софт чел без читов ты 0",
		"Чел ты без читов 0",
		"Го 1 на 1 или зассал?Точно ты же до 1 считать не умееш...",
		"Мы в НОНРП Зоне как бы да чел отлетаеш",
		"Найс баг абуз чел папа жива?",
		"Ало скорая тут такой случай шкiла упала в месорубку",
		"Откисай молодой!",
		"говори буду плохо говорить буду сосать, буду плохо сосать буду пересасывать",
		"долбаеб иди башмачки в сундучок школьный собирай",
		"ботинок ебаный чо слетел",
		"братик маме привет передай",
		"не противник",
		"а ты че клоун???",
		"я обоссал тебя (",
		"ты че там отлетел то?",
		"Я твою маму дуже сильно поважаю , нехай береже її Степан Бендера",
		"упал хуета ебаная , но в боди забрал да похуй все равно упал",
		"ливай с хвх (",
		"до связи башмак",
		"нищета глупейшая играть учись",
		"опущен сын твари",
		"сразу видно кфг иссуе мб конфиг у меня прикупишь ?",
		"животное аддон скачай а то падаешь",
		"оттарабанен армянская королева",
		"сука не позорься и ливни",
		"улетел тапочек ебаный",
		"единицей свалился фуфлыжник",
		"Вот тебе паяльник , запаяй себе ебальник",
		"зачем ты играешь тут безмозглый", "иди кумыса попей очередняра",
		"Ты как кофе , 3 в одном - пидр , чмошник и гандон",
		"откисай сочняра",
		"АХАХА ЕБАТЬ У ТЕБЯ ЧЕРЕПНАЯ КОРОБКА ПРЯМ КАК [XML-RPC] No-Spread 24/7 | aim_ag_texture_2 ONLY!",
		"на мыло и веревку то деньги есть????",
		"ИЩИ СЕБЯ НА pornoeb.cc/so4niki",
		"свежий кабанчик",
		"до связи на подскоке кабанчик",
		"скажи маме сухарики купить долбаеб",
		"ебать ты красиво на бутылку присел , тебе дать альт ?",
		"Извини дорогая , не хотел на лицо",
		"прости что без смазки)",
		"алло это скорая? тут такая ситуация парню который упал нужна скорая)",
		"ало ты мапу лузаешь , дура очнись",
		"ЕБУЧЕСТЬ ВТОРОГО РАЗРЯДА ВЫДВИЖЕНЕЦ ОТКИС",
		"але , а противники то где???",
		"ты по легиту играешь ?",
		"ХУЕПРЫГАЛО ТУСОВОЧНОЕ КУДА ПОЛЕТЕЛО",
		"ты куда жертва козьего аборта",
		"iq?", "·٠●•۩۞۩ОтДыХаЙ (ٿ) НуБяРа۩۞۩•●٠·",
		"ты то куда лезешь сын фантомного стационарного спец изолированого металлформовочного механизма",
		"╭∩╮( ⚆ ʖ ⚆)╭∩╮ ДоПрыГался(ت)ДрУжоЧеК",
		"Тебе в ротик или на животик ?"
	},
	["English HvH"] = {
		"Bro imagine resolving in gmod",
		"ez",
		"loser",
		"rekt",
		"nice move",
		"what the fuck are you using %s",
		"noob",
		"did you get that garbage from the steam workshop?",
		"you got fucked in the ass",
		"get fucking raped",
		"%s can drink my fucking piss",
		"you suck shit gay nigger",
		"you should eat my shit",
		"you got shafted by my large penis, %s",
		"%s is getting fucked by an aimbot",
		"%s is getting fucking murdered",
		"you're so shit at this game, quit already",
		"drink my dog's piss faggot",
		"hey don't cry bro, you need a tissue?",
		"you're so fucking gay",
		"you're the reason why equal rights don't exist, %s",
		"%s is radiating big faggot energy",
		"hurr durr stop cheating in an ancient video game!!!",
		"stop being such a spastical retard already",
		"you're more braindead than kim jong un after his surgery",
		"you're a furfag and should not be proud,%s",
		"%s is getting dominated by me, aka god",
		"you live in a fucking dirty hut,retarded african boy",
		"i bet you're literally fucking black",
		"%s is a gay autistic nigger with no privileges",
		"%s is being searched for by the fbi",
		"%s literally fucking died in gmod",
		"you're ultra retarded, kid",
		"you need a tissue, little faggot?",
		"%s should get killed by me once again",
		"please die more,you're feeding my addiction",
		"%s is a retard bot",
		"you're so much of a loser,get a fucking life and stop playing this shit game kid",
		"virgin lol get good",
		"fucking coomer,go wash your crusty underwear you filth",
		"%s got cucked",
		"%s is dominated by pure fucking skill",
		"you are a big noob",
		"i can't wait to headshot you irl, %s",
		"you smelly homeless nigger",
		"%s still believes that god and santa exist lol",
		"bruh you really do be crying at a game",
		"please stop doing what you're doing and kill yourself",
		"%s lives in america",
		"you are a deformed fetus",
		"%s is ugly as shit fr tho",
		"you're cringe, stop doing this shit",
		"%s, you look like you died",
		"fucking putrid fuck,kill yourself",
		"%s is a trash cheater",
		"%s is a normie",
		"smelly fucker",
		"%s is a dickless prick",
		"%s is gay",
		"%s does not get any pussy",
		"you're too stupid to be considered human",
		"%s is a furry",
		"%s is a waste of human flesh",
		"i bet you won't be able to kill me even with hacks",
		"%s, men are the fuck. you are not the fuck. you are not men",
		"%s is a failed abortion",
		"%s fucking died",
		"%s plays with his dick for fun",
		"play with my stinky fat throbbing cock you gay faggot",
		"stop using hacks you cringe skid!!!",
		"%s uses cancer shit cheats!!",
		"you show all of the signs of mental retardation",
		"please just quit the game already",
		"%s is a %s",
		"shut the fuck up and die",
		"nigger lol"
	},
	["English HvH 2"] = {
		"sick resolver",
		"sick fakelag",
		"sick antiaim",
		"sick aimbot",
		"sick bhop",
		"sick spinbot",
		"nice aimware paste",
		"nice memeware paste",
		"what the fuck are you using lol",
		"sick cfg",
		"it must be a cfg issue, right?",
		"it must be a brain issue",
		"fix your *DEAD*",
		"BRUH",
		"ez",
		"ezz",
		"what are you shooting at lmao",
		"ez retard",
		"ez nn",
		"lol why so ez",
		"lol ez",
		"bro imagine resolving in gmod",
		"nice fucking engine prediction",
		"sick enginepred, you sell???",
		"nice brain, you sell???",
		"nice cfg, you sell???",
		"nice keybinds, you sell???",
		"nice aimware paste, you sell???",
		"nice free the skids paste",
		"nice internet",
		"nice computer",
		"sick steeringwheel assistance",
		"nice steeringwheel assistance",
		"insane vip hack",
		"insane aimware paste",
		"crazy aimware paste",
		"i cant tell if you're joking",
		"too fucking easy",
		"nice playstyle",
		"nice chromosome count",
		"easiest kill of my life",
		"nice fucking antiaim",
		"consider suicide",
		"imagine the only thing you eat being bullets man",
		"ez idiot",
		"is this methamphetamine???",
		"is this idiotbox???",
		"is this aoshax???",
		"is this rijin???",
		"no spin no win",
		"no backtrack no win",
		"ez baim retard",
		"mind enabling your antiaim",
		"mind enabling your fakelag",
		"ming enabling your aimbot",
		"nice keybinds",
		"wtf you died when i was afk",
		"even smeghack will tap you LMAO",
		"green green what's your problem green me say alone ramp me say alone ramp",
		"so ez"
	}
}

--[[
do
	local opezdal = file.Read("menuhook/loles.txt", "MOD")
	if opezdal then
		for k, v in ipairs(opezdal:Split("\n")) do
			if #v < 126 then
				table.insert(killrow_messages["Opezdal"], v)
			end
		end

		print(#killrow_messages["Opezdal"], "Opezdals loaded")
	end

	local unizh = file.Read("menuhook/lolmems.txt", "MOD")
	if unizh then
		for k, v in ipairs(unizh:Split("\n")) do
			if #v < 126 then
				table.insert(killrow_messages["Unizhenie"], v)
			end
		end

		print(#killrow_messages["Unizhenie"], "Unizhalok loaded")
	end
end
--]]

do

local hitboxes = {}

gameevent.Listen("entity_killed")
AddHook("entity_killed", function(data)
	local victim, attacker = Entity(data.entindex_killed), Entity(data.entindex_attacker)
	if attacker == me and attacker != victim and victim:IsPlayer() then
		if settings.Misc.KillSound then
			surface.PlaySound("buttons/" .. settings.Misc.KillSoundSnd .. ".wav")
		end

		if settings.Misc["Chat killrow"] then
			local msgs = killrow_messages[settings.Misc.KillrowMode]
			local msg = msgs[mrandom(#msgs)]
			if msg:find("%s") then
				msg = msg:format(victim:Nick())
			end

			if DarkRP then
				msg = "/ooc " .. msg
			end

			RunConsoleCommand("say", msg)
		end
	end

	if /*attacker:IsPlayer() and*/ victim == me then
		local hitboxData = {}
		for hitboxGroupId = 0, me:GetHitboxSetCount() - 1 do
			for hitboxId = 0, me:GetHitBoxCount(hitboxGroupId) - 1 do
				local boneId = me:GetHitBoxBone(hitboxId, hitboxGroupId)
				local boneMat = me:GetBoneMatrix(boneId)
				local bonePos = boneMat:GetTranslation()
				local boneAng = boneMat:GetAngles()

				local mins, maxs = me:GetHitBoxBounds(hitboxId, hitboxGroupId)

				hitboxData[#hitboxData + 1] = {
					pos = bonePos,
					angles = boneAng,
					mins = mins, maxs = maxs
				}
			end
		end

		hitboxes[#hitboxes + 1] = {
			createdTime = CurTime(),
			hitboxData = hitboxData
		}
	end
end)

AddHook("PostDrawOpaqueRenderables", function()
	render.SetColorMaterial()

	local curTime = CurTime()
	for i = #hitboxes, 1, -1 do
		local hitboxSet = hitboxes[i]
		if (curTime - hitboxSet.createdTime) > 5 then
			table.remove(hitboxes, i)
			continue
		end

		for j = 1, #hitboxSet.hitboxData do
			local hitboxData = hitboxSet.hitboxData[j]
			render.DrawBox(hitboxData.pos, hitboxData.angles, hitboxData.mins, hitboxData.maxs)
		end
	end
end)

end

gameevent.Listen("player_hurt")
AddHook("player_hurt", function(data)
	local victim, attacker = Player(data.userid), Player(data.attacker)
	if attacker == me and victim != attacker then
		if settings.Misc.HurtSound then
			surface.PlaySound("buttons/" .. settings.Misc.HurtSoundSnd .. ".wav")
		end

		victim.sw_aim_shots = (victim.sw_aim_shots or 0) - 1

		if settings.HvH.Resolver and (settings.HvH.ResolverMode == "StatAbs" or settings.HvH.ResolverMode == "StatRel") then
			local yaw = math.floor(victim:GetRenderAngles().y)
			victim.sw_resolve_stats[yaw] = victim.sw_resolve_stats[yaw] + 1

			print(victim:Nick(), "STATS:")
			PrintTable(victim.sw_resolve_stats)
		end
	end
end)

do

local thickness = SCRW * 0.0015
local length = SCRW * 0.009
local crs_color = Color(175, 175, 175)
local color_red = Color(255, 0, 0)
local color_green = Color(0, 255, 0)
local color_yellow = Color(245, 245, 0)

local aspectratio = SCRW / SCRH
local verticalfov = mrad(74)
local realfov = 2 * math.atan(math.tan(verticalfov * 0.5) * aspectratio) * 0.5

local mtan = math.tan

AddHook("HUDPaint", function()
	if settings.Visuals.EnabledP then
		ESP()
	end

	if settings.Visuals["FOV cone"] then
		local radius = (mtan(mrad(settings.Aimbot["FOV cone"]) * 0.5) / mtan(realfov)) * SCRW

		surface_SetDrawColor(color_white)
		surface_DrawCircle(SCRW * 0.5, SCRH * 0.5, radius)
	end

	local cw, ch = SCRW * 0.5, SCRH * 0.5
	if settings.Visuals.Crosshair2D then
		surface_SetDrawColor(crs_color)
		surface_DrawRect(cw - length * 0.5, ch - thickness * 0.5, length, thickness)
		surface_DrawRect(cw - thickness * 0.5, ch - length * 0.5, thickness, length)
	end

	local y = 0

	local function DrawCenterText(text, color, font)
		surface_SetTextColor(color)
		surface_SetTextPos(cw, ch + y)
		surface_SetFont("DebugFixed")
		surface_DrawText(text)

		local _, th = surface_GetTextSize(text)
		y = y + th
	end

	if settings.Visuals.AimTarget and IsValid(aimply) then
		DrawCenterText("TARGET: " .. aimply:Nick(), color_red, "DebugFixed")
	end

	if settings.HvH.Warp or settings.HvH.Doubletap or settings.HvH["Warp on peek"] then
		local text = "CHARGED"
		local color
		if tickbaseshiftcharge then
			color = color_green
			text = "CHARGED"
		elseif tickbaseshiftcharging then
			color = color_yellow
			text = "CHARGING"
		else
			color = color_red
			text = "UNCHARGED"
		end

		DrawCenterText(text, color, "DebugFixed")
		DrawCenterText(("SHIFT %d (%d)"):format(ticksshifted, ticksshiftedtotal), color, "DebugFixed")
	end
end)

end

AddHook("EntityFireBullets", function(ply, data)
	local spread = data.Spread * -1
	local weap = ply:GetActiveWeapon()
	if IsValid(weap) and spread != vector_origin then
		weapcones[weap:GetClass()] = spread
	end
end)

-- not needed for CHLClient createmove
AddHook("CalcView", function(_, origin, angles)
	local thirdperson = settings.Visuals.Thirdperson and keybinds.Visuals.ThirdpersonKey

	local view = {}
	view.drawviewer = thirdperson
	view.angles = settings.Aimbot.Silent and silentangles or angles
	if thirdperson then
		local cameraPos = origin - ((settings.Aimbot.Silent and silentangles or angles):Forward() * settings.Visuals.ThirdpersonDist)
		local trace = TraceLine({
			start = origin,
			endpos = cameraPos,
			filter = me,
			mask = MASK_OPAQUE
		})

		view.origin = trace.HitPos
	end

	view.fov = settings.Visuals.Fov

	return view
end)

-- same as CalcView

--[[
AddHook("CalcViewModelView", function(_, _, _, _, pos, ang)
	if settings.Aimbot.Silent then
		return gorigin or pos, silentangles
	end
end)
--]]

do

local drawing = false
AddHook("PreDrawViewModel",  function(vm, ply, weap)
	local fov = settings.Visuals["Viewmodel fov"]
	if ply != me or fov == 75 or drawing then return end

	cam.IgnoreZ(true)
	cam.Start3D(nil, nil, fov)
	drawing = true
	vm:DrawModel()
	drawing = false
	cam.End3D()
	cam.IgnoreZ(false)

	return true
end)

end

do

local fakelagmodel = NewFakeModel(me, RENDERGROUP_TRANSLUCENT)
local realmodel = NewFakeModel(me, RENDERGROUP_OPAQUE)
local fakemodel = NewFakeModel(me, RENDERGROUP_OPAQUE)

local realangcolor = Color(238, 158, 29)
local fakeangcolor = Color(22, 196, 187)
local lbyangcolor = Color(25, 56, 231)

local bulletImpacts = {}
local numBulletImpacts = 0
local numPlayerHurts = 0
samoware.AddHook("OnImpact", "samoware", function(data)
	local bestPlayer = nil
	local bestDistance = math.huge
	for _, ply in ipairs(player.GetAll()) do
		local distance = ply:EyePos():DistToSqr(data.m_vStart)
		if distance < bestDistance then
			bestPlayer = ply
			bestDistance = distance
		end
	end

	bulletImpacts[#bulletImpacts + 1] = {
		shootTime = CurTime(),
		attacker = bestPlayer,

		startPos = data.m_vStart,
		endPos = data.m_vOrigin,
		hitbox = data.m_nHitbox,
	}

	if bestDistance < 128 then
		if bestPlayer == me then
			numBulletImpacts = numBulletImpacts + 1
		else
			bestPlayer.sw_backshoot_data = {
				headPos = bestPlayer.sw_cur_headpos,
				simTime = bestPlayer.sw_cur_simtime
			}
		end
	end
end)

hook.Remove("CreateMove", "testim", function(cmd)
	if cmd:CommandNumber() == 0 then return end
	print(Entity(2):EyeAngles())
end)

gameevent.Listen("player_hurt")
AddHook("player_hurt", function(data)
	local victim, attacker = Player(data.userid), Player(data.attacker)
	if attacker == me and victim != attacker then
		numPlayerHurts = numPlayerHurts + 1
	end
end)

AddHook("HUDPaint", function()
	if !settings.Visuals.HitMiss then return end

	draw.SimpleText(("Hits: %d"):format(numPlayerHurts), "TargetID", 0, 128)
	draw.SimpleText(("Shots: %d"):format(numBulletImpacts), "TargetID", 0, 128 + 14)
	draw.SimpleText(("Hit/Miss: %.1f%%"):format(numPlayerHurts / numBulletImpacts * 100), "TargetID", 0, 128 + 14 * 2)
end)

AddHook("PostDrawOpaqueRenderables", function()
	if settings.Visuals.AntiAimChams and settings.Visuals.Thirdperson and keybinds.Visuals.ThirdpersonKey then
		render.MaterialOverride(chamsmat)

		UpdateFakeModel(fakemodel, fakeangles)
		UpdateFakeModel(realmodel, realangles)

		local rl = realmodel.model
		local fk = fakemodel.model

		render.SetColorModulation(0.95, 0.61, 0.07)
		rl:DrawModel()

		render.SetColorModulation(0.15, 0.61, 0.57)
		fk:DrawModel()

		render.MaterialOverride()
	end

	if settings.Visuals.AntiAimLines then
		local pos = me:GetPos()
		cam.IgnoreZ(true)
		cam.Start3D2D(pos, Angle(0, realangles.y + 45, 0), 1)
			surface_SetDrawColor(realangcolor)
			surface_DrawLine(0, 0, 25, 25)
		cam.End3D2D()

		cam.Start3D2D(pos, Angle(0, fakeangles.y + 45, 0), 1)
			surface_SetDrawColor(fakeangcolor)
			surface_DrawLine(0, 0, 25, 25)
		cam.End3D2D()

		local lby = samoware.GetCurrentLBY(me:EntIndex())
		cam.Start3D2D(pos, Angle(0, lby + 45, 0), 1)
			surface_SetDrawColor(lbyangcolor)
			surface_DrawLine(0, 0, 25, 25)
		cam.End3D2D()

		cam.IgnoreZ(false)
	end

	if settings.Visuals.Crosshair3D then
		local tr = me:GetEyeTrace()

		cam.Start3D2D(tr.HitPos, tr.HitNormal:Angle() - Angle(90, 0, 0), 1)
		surface.SetDrawColor(crs_color)
		local SCRW, SCRH = ScrW(), ScrH()
		local cw, ch = SCRW * 0.5, SCRH * 0.5

		surface_SetDrawColor(crs_color)
		surface_DrawRect(-length * 0.5, -thickness * 0.5, length, thickness)
		surface_DrawRect(-thickness * 0.5, -length * 0.5, thickness, length)
		cam.End3D2D()
	end

	if settings.Visuals.Tracers then
		for k, v in ipairs(player.GetAll()) do
			if v == me then continue end

			local tr = v:GetEyeTrace()
			local start, hitpos = tr.StartPos, tr.HitPos

			render.DrawLine(start, hitpos)
		end
	end

	if settings.Visuals.BulletTracers then
		local curTime = CurTime()
		local dieTime = settings.Visuals["Tracers die time"]
		for i = #bulletImpacts, 1, -1 do
			local impact = bulletImpacts[i]
			if (curTime - impact.shootTime) > dieTime then
				table.remove(bulletImpacts, i)
				continue
			end

			render.DrawLine(impact.startPos, impact.endPos, color_white, true)
		end
	end
end)

AddHook("HUDPaint", function()
	if !settings.Visuals.AntiAimLines then return end

	local rl, fk, lby = Angle(0, realangles.y, 0), Angle(0, fakeangles.y, 0), Angle(0, samoware.GetCurrentLBY(me:EntIndex()), 0)

	local pos = me:GetPos()
	local rlpos = (pos + rl:Forward() * 36):ToScreen()
	local fkpos = (pos + fk:Forward() * 36):ToScreen()
	local lbypos = (pos + lby:Forward() * 36):ToScreen()

	-- draw real
	surface_SetFont("swcc_text_visualize")

	local tx, ty = surface_GetTextSize("real")
	surface_SetTextPos(rlpos.x - tx * 0.5, rlpos.y - ty * 0.5)

	surface_SetTextColor(realangcolor)
	surface_DrawText("real")

	-- draw fake
	tx, ty = surface_GetTextSize("fake")
	surface_SetTextPos(fkpos.x - tx * 0.5, fkpos.y - ty * 0.5)

	surface_SetTextColor(fakeangcolor)
	surface_DrawText("fake")

	-- draw LBY
	tx, ty = surface_GetTextSize("LBY")
	surface_SetTextPos(lbypos.x - tx * 0.5, lbypos.y - ty * 0.5)

	surface_SetTextColor(lbyangcolor)
	surface_DrawText("LBY")
end)

AddHook("PrePlayerDraw",function(ply)
	if ply == me and settings.HvH.Visualize then
		return true
	end
end)

AddHook("PostDrawTranslucentRenderables", function()
	if bSendPacket then
		UpdateFakeModel(fakelagmodel)
	end

	render.MaterialOverride(chamsmat_transparent)

	if settings.Visuals.FakeLagChams and settings.Visuals.Thirdperson and keybinds.Visuals.ThirdpersonKey then
		render.SetBlend(0.7)
		render.SetColorModulation(0.25, 0.25, 0.7)
		fakelagmodel.model:DrawModel()
	end

	render.MaterialOverride()
end)

end

do
	for _, ply in ipairs(player.GetAll()) do
		ply.sw_prev_pos = Vector()
		ply.sw_cur_pos = ply:GetPos()

		ply.sw_prev_ang = Angle()
		ply.sw_cur_ang = ply:EyeAngles()

		ply.sw_prev_headpos = Vector()
		ply.sw_cur_headpos = GetBone(ply, "Center", "Head")[1]

		ply.sw_cur_simtime = samoware.GetSimulationTime(ply:EntIndex())
		ply.sw_prev_simtime = 0

		ply.sw_aim_shots = 0

		ply.sw_resolve_stats = {}
		for i = -180, 180, 45 do
			ply.sw_resolve_stats[i] = 1
		end
	end
end

gameevent.Listen("player_spawn")
AddHook("player_connect_client", function(ply)
	ply.sw_prev_pos = Vector()
	ply.sw_cur_pos = v:GetPos()

	ply.sw_prev_ang = Angle()
	ply.sw_cur_ang = ply:EyeAngles()

	ply.sw_prev_headpos = Vector()
	ply.sw_cur_headpos = GetBone(ply, "Center", "Head")[1]

	ply.sw_cur_simtime = samoware.GetSimulationTime(v:EntIndex())
	ply.sw_prev_simtime = 0

	ply.sw_resolve_stats = {}
	for i = -180, 180, 45 do
		ply.sw_resolve_stats[i] = 1
	end
end)

do

local navalnytext = {
	"Навальный топчик",
	"Навальный топчик",
	"Навальный топчик",
	"Навальный топчик,за него Тверскую топчем",
	"Навальный топчик,за него Тверскую топчем",
	"Нью Бэланс кеды, прилипли к подошве гетры",
	"Но сегодня в центре в них устроим веселье",
	"Мы отсюда не свалим",
	"Все кто дома - не с нами",
	"Мы тут просто гуляем",
	"В нашем сердце весна В нашем сердце весна",
	"В нашем сердце весна",
	"Навальный топчик,за него Тверскую топчем",
	"Навальный топчик,скажем громче",
	"Навальный топчик,за него Тверскую топчем",
	"Навальный топчик,",
	"Навальный топчик,за него Тверскую топчем",
	"Тверскую топчем",
	"Вокруг так много космонавтов",
	"МКС полицейский пазик",
	"Лица скрывают каски,маски",
	"Становиться опасно,но",
	"Мы устроим пляски",
	"Дружно,под эти песни",
	"Вся Тверская в курсе",
	"Вся Тверская денсит",
	"Тверская денсит",
	"Тверская денсит",
	"Денсит",
	"Навальный топчик,за него Тверскую топчем",
	"Навальный топчик,скажем громче",
	"Навальный топчик,за него Тверскую топчем",
	"Навальный топчик,",
	"Навальный топчик,за него Тверскую топчем",
	"Тверскую топчем",
	"Навальный топчик",
	"15 суток, нам нет места от скуки",
	"Ждем когда вернешься, Навальный Леша",
	"Время летит быстро,скоро новая вписка",
	"Мы не пойдем на пары если,Навальный с нами",
	"Навальный с нами,давай с нами",
	"Навальный с нами, пойдем тусить с нами",
	"Навальный с нами, давай с нами",
	"Давай с нами, пойдем тусить с нами",
	"Этому городу нужен герой",
	"Леша Навальный, мы с тобой",
	"Этой стране нужен герой",
	"Леша Навальный, мы с тобой",
	"Этой планете нужен герой",
	"Леша Навальный, мы с тобой",
	"Этой Вселенной нужен герой",
	"Леша Навальный, мы с тобой",
	"Этой Вселенной нужен герой",
	"Леша Навальный, мы с тобой",
	"Леша Навальный, мы с тобой"
}

local navalnyidx = 0
local nextnavalny = -1

-- rebro pidor
local nextact = 0

local boost_cvars = {
	["gmod_mcore_test"] = 1,
	["r_3dsky"] = 0,
	["gmod_mcore_test"] = 1,
	["snd_mix_async"] = 1,
	["cl_threaded_bone_setup"] = 1,
	["cl_threaded_client_leaf_system"] = 1,
	["r_queued_ropes"] = 1,
	["mat_queue_mode"] = 2,
	["r_threaded_renderables"] = 1,
	["r_threaded_particles"] = 1,
	["r_threaded_client_shadow_manager"] = 1,
	["cl_forcepreload"] = 1,
	["r_flex"] = 1,
	["r_drawflecks"] = 1,
	["r_fastzreject"] = -1,
	["r_decals"] = 1024,
	["cl_ejectbrass"] = 0,
	["rope_shake"] = 0,
	["r_WaterDrawReflection"] = 1,
	["r_WaterDrawRefraction"] = 1
}

local old_cvars = {}
for k, v in pairs(boost_cvars) do
	old_cvars[k] = GetConVar(k):GetFloat()
end

local fps_boosted = false
local fps_notboosted = false

local c_net_fakelag = GetConVar("net_fakelag")
local c_net_fakejitter = GetConVar("net_fakejitter")
local c_net_fakeloss = GetConVar("net_fakeloss")
local c_host_timescale = GetConVar("host_timescale")

local c_cl_extrapolate = GetConVar("cl_extrapolate")
local c_cl_interp = GetConVar("cl_interp")
local c_cl_interp_ratio = GetConVar("cl_interp_ratio")
AddHook("Think", function()
	aaply = GetPlayers(settings.HvH["AA ply mode"], 0, 1, false)
	if aaply then
		aaply = aaply[1][1]
	end

	if settings.Misc.FpsBoost then
		if !fps_boosted then
			for k, v in pairs(boost_cvars) do
				RunConsoleCommand(k, v)
			end

			fps_boosted = true
			fps_notboosted = false
		end
	else
		if !fps_notboosted then
			for k, v in pairs(old_cvars) do
				RunConsoleCommand(k, v)
			end

			fps_boosted = false
			fps_notboosted = true
		end
	end

	if settings.Aimbot.FixErrors then
	--for k, v in ipairs(player.GetAll()) do
		--	if v:GetModel() == "models/error.mdl" then
		--		v:SetModel("models/player/breen.mdl")
		--	end
		--end
	end

	if settings.Misc.AutoNavalny and CurTime() > nextnavalny then
		RunConsoleCommand("say", navalnytext[navalnyidx + 1])
		navalnyidx = (navalnyidx + 1) % #navalnytext
		nextnavalny = CurTime() + 1
	end

	if settings.HvH["Dance"] and me:Alive() and !me:IsPlayingTaunt() and CurTime() > nextact then
		local act = settings.HvH["DanceMode"]
		if act == "random" then
			act = dancemodes[mrandom(2, #dancemodes)]
		end

		RunConsoleCommand("act", act)
		nextact = CurTime() + 0.3
	end

	if settings.HvH.Cvar3 then
		local net_fakelag = 0
		local net_fakejitter = 0
		local net_fakeloss = 0
		local host_timescale = 1

		if settings.HvH["Manip. enabled"] then
			net_fakelag = settings.HvH.net_fakelag
			net_fakejitter = settings.HvH.net_fakejitter
			net_fakeloss = settings.HvH.net_fakeloss
			host_timescale = settings.HvH.host_timescale
		end

		c_net_fakelag:SetValue(net_fakelag)
		c_net_fakejitter:SetValue(net_fakejitter)
		c_net_fakeloss:SetValue(net_fakeloss)
		c_host_timescale:SetValue(host_timescale)

		if settings.HvH.NoInterp then
			c_cl_extrapolate:SetValue(0)
			samoware.SetEnableInterpolation(false)
		else
			c_cl_extrapolate:SetValue(1)
			samoware.SetEnableInterpolation(true)
		end
	end

end)

local mats = {}
for k, v in pairs(handchamsmaterials) do
	if v == "None" then continue end
	v = "models/" .. v
	mats[v] = Material(v)
end

local drawing = false
AddHook("PreDrawPlayerHands", function(hands, vm, ply, weapon)
	if drawing or ply != me or !IsValid(hands) then return end

	local mat = settings.Visuals.HandChams
	if mat == "None" then
		return
	else
		mat = "models/" .. mat
	end

	local color = colors.handchams
	local r, g, b = color.r / 255, color.g / 255, color.b / 255

	render.MaterialOverride(mats[mat])
	render.SetColorModulation(r, g, b)

	drawing = true
	hands:DrawModel()
	drawing = false

	render.MaterialOverride()

	return true
end)

end

do

local FRAME_START = 0
local FRAME_NET_UPDATE_START = 1
local FRAME_NET_UPDATE_POSTDATAUPDATE_START = 2
local FRAME_NET_UPDATE_POSTDATAUPDATE_END = 3
local FRAME_NET_UPDATE_END = 4
local FRAME_RENDER_START = 5
local FRAME_RENDER_END = 6

local backtrackRecords = {}

local function BacktrackRecord(ply)
	/*
	local deadtime = CurTime() - settings.HvH.BacktrackAmount / 1000

	local records = backtrackRecords[ply]
	if !records then
		records = {}
		backtrackRecords[ply] = records
	end

	local i = 1
	while i < #records do
		local record = records[i]

		if record.simulationtime < deadtime then
			table.remove(records, i)
			i = i - 1
		end

		i = i + 1
	end

	if !ply:Alive() then return end

	local simulationtime = samoware.GetSimulationTime(ply:EntIndex())
	local len = #records
	local simtimechanged = true
	if len > 0 then
		simtimechanged = records[len].simulationtime < simulationtime
	end

	if ply.sw_cur_pos:DistToSqr(ply.sw_prev_pos) < 4096 and simtimechanged then
		local layers = {}
		for i = 0, 13 do
			if ply:IsValidLayer(i) then
				layers[i] = {
					cycle = ply:GetLayerCycle(i),
					sequence = ply:GetLayerSequence(i),
					weight = ply:GetLayerWeight(i)
				}
			end
		end

		records[len + 1] = {
			simulationtime = samoware.GetSimulationTime(ply:EntIndex()),
			angles = ply:GetAngles(),
			origin = ply.sw_cur_pos,
			maxs = ply:OBBMaxs(),
			mins = ply:OBBMins(),
			sequence = ply:GetSequence(),
			cycle = ply:GetCycle(),
			layers = layers
		}
	end
	*/
end

local nulAng = Angle(0, 0, 0)
samoware.AddHook("PostFrameStageNotify", "samoware", function(stage)
	if stage == FRAME_NET_UPDATE_POSTDATAUPDATE_END then
		local realtime = RealTime()
		for k, v in ipairs(player.GetAll()) do
			if !v.sw_last_dormant then
				v.sw_last_dormant = 0
			end

			if v == me then
				continue
			elseif v:IsDormant() and v.sw_last_dormant == 0 then
				v.sw_last_dormant = realtime
				continue
			end

			if !v:IsDormant() then
				v.sw_last_dormant = 0
			end

			if settings.HvH.Backtrack then BacktrackRecord(v) end

			local cur_simtime = samoware.GetSimulationTime(v:EntIndex())
			if v.sw_cur_simtime != cur_simtime then
				v.sw_prev_pos = v.sw_cur_pos
				v.sw_cur_pos = v:GetNetworkOrigin()

				v.sw_prev_ang = v.sw_cur_ang
				v.sw_cur_ang = v:EyeAngles()

				v.sw_prev_headpos = v.sw_cur_headpos
				v.sw_cur_headpos = GetBone(v, "Center", "Head")[1]

				v.sw_prev_simtime = v.sw_cur_simtime
				v.sw_cur_simtime = cur_simtime
			end
		end
	elseif stage == FRAME_RENDER_START then
		for k, v in ipairs(player.GetAll()) do
			if v == me or v:IsDormant() or !v.sw_cur_pos or !v.sw_prev_pos then continue end

			-- extrapolate noobs breaking lc
			if false then --settings.HvH.LagFix and v.sw_cur_pos:DistToSqr(v.sw_prev_pos) > 4096 then
				local predTime = samoware.GetLatency(0) + samoware.GetLatency(1)
				samoware.StartSimulation(v:EntIndex())

				for tick = 1, TIME_TO_TICKS(predTime) do
					samoware.SimulateTick()
				end

				local data = samoware.GetSimulationData()
				v:SetRenderOrigin(data.m_vecAbsOrigin)
				v:SetNetworkOrigin(data.m_vecAbsOrigin)

				samoware.FinishSimulation()
			elseif settings.HvH.Forwardtrack then
				local predTime = (samoware.GetLatency(0) + samoware.GetLatency(1)) * settings.HvH.ForwardtrackAmount
				samoware.StartSimulation(v:EntIndex())

				local prevPos = v:GetNetworkOrigin()
				for tick = 1, TIME_TO_TICKS(predTime) do
					samoware.SimulateTick()

					local data = samoware.GetSimulationData()
					debugoverlay.Line(prevPos, data.m_vecAbsOrigin, 0.1, color_white, true)

					prevPos  = data.m_vecAbsOrigin
				end

				local data = samoware.GetSimulationData()
				v:SetRenderOrigin(data.m_vecAbsOrigin)
				v:SetNetworkOrigin(data.m_vecAbsOrigin)

				samoware.FinishSimulation()
			end

			if settings.HvH.Resolver and v.sw_aim_shots then
				if settings.HvH.ResolverMode == "Absolute" or settings.HvH.ResolverMode == "Relative" then
					local bruteforceAngles = {-90, 0, 90, 180, -180, 180, 90, 0, -90}
					local headBruteforceAngles = {45, 0, -45}
					local angles = Angle()
					angles.y = bruteforceAngles[v.sw_aim_shots % #bruteforceAngles + 1]

					if settings.HvH.ResolverMode == "Relative" then
						angles.y = angles.y + v:EyeAngles().y
					end

					v:SetRenderAngles(angles)

					if settings.HvH.ResolveFakePitch and v:EyeAngles().p > 90 then
						v:SetPoseParameter("aim_pitch", -89)
						v:SetPoseParameter("head_pitch", -89)
					end

					local headAngle = headBruteforceAngles[v.sw_aim_shots % #headBruteforceAngles + 1]
					v:SetPoseParameter("aim_yaw", headAngle)
					v:SetPoseParameter("head_yaw", headAngle)
				elseif settings.HvH.ResolverMode == "StatAbs" or settings.HvH.ResolverMode == "StatRel" then
					if !v.sw_resolve_stats then
						v.sw_resolve_stats = {}
						for i = -180, 180, 45 do
							v.sw_resolve_stats[i] = 1
						end
					end

					math.randomseed(v:EntIndex() + v.sw_aim_shots + 0xDEAD)

					local weightSum = 0
					for _, weight in pairs(v.sw_resolve_stats) do
						weightSum = weightSum + weight
					end

					local sel = mrandom(weightSum)
					local selYaw
					for yaw, weight in pairs(v.sw_resolve_stats) do
						sel = sel - weight
						if sel <= 0 then
							selYaw = yaw
							break
						end
					end

					local angles = Angle()
					angles.r = 0
					angles.y = selYaw

					v:SetRenderAngles(angles)

					if settings.HvH.ResolveFakePitch and v:EyeAngles().p > 90 then
						v:SetPoseParameter("aim_pitch", -89)
						v:SetPoseParameter("head_pitch", -89)
					end

					v:SetPoseParameter("aim_yaw", 0)
					v:SetPoseParameter("head_yaw", 0)
				end

				-- Fix legs
				local angles = v:GetRenderAngles()
				local velocity = v:GetVelocity()
				local velocityYaw = mNormalizeAng(angles.y - math.deg(math.atan2(velocity.y, velocity.x)))
				local playbackRate = v:GetPlaybackRate()
				local moveX = mcos(mrad(velocityYaw)) * playbackRate
				local moveY = -msin(mrad(velocityYaw)) * playbackRate

				v:SetPoseParameter("move_x", moveX)
				v:SetPoseParameter("move_y", moveY)

				v:InvalidateBoneCache()
			end
		end
	end
end)

end

AddHook("PrePlayerDraw", function(ply)
	if settings.HvH.ActResolver and (ply != me or settings.HvH.AA == "Anti BruteForce") then
		ply.ChatGestureWeight = 0

		for i = 0, 13 do
			if ply:IsValidLayer(i) then
				local seqname = ply:GetSequenceName(ply:GetLayerSequence(i))
				if seqname:StartWith("taunt_") or seqname:StartWith("gesture_") then
					ply:SetLayerDuration(i, 0.001)
					break
				end
			end
		end
	end

	if settings.Visuals.AntiAimChams and ply == me then
		return true
	end
end)

AddHook("PreRender", function()
	if settings.Visuals.Fullbright then
		render.SetLightingMode(1)
	end
end)

AddHook("RenderScreenspaceEffects", function()
	if !settings.Visuals.Fullbright then return end
	render.SetLightingMode(0)
end)

AddHook("HUDShouldDraw", function(name)
	if settings.Visuals.Crosshair and name == "CHudCrosshair" then return false end
end)

local function SetTyping(cmd)
	-- easychat kostili
	if EasyChat then
		if cmd:CommandNumber() % 32 == 0 then
			net.Start("EASY_CHAT_START_CHAT")
			net.WriteBool(true)
			net.SendToServer()
		end

		return
	end

	samoware.SetTyping(cmd, true)
end

do

-- Initialize player variables
for k, v in ipairs(player.GetAll()) do
	v.sw_cur_pos = v:GetPos()
	v.sw_prev_pos = v:GetPos()
	v.sw_cur_simtime = 0
	v.sw_prev_simtime = 0
	v.sw_last_dormant = 0
	v.sw_aim_shots = 0
end

local prevang
local autopistol = false

local positions = {}
hook.Add("HUDPaint", "dt", function()
	surface.SetDrawColor(255, 255, 255)
	for i = 1, #positions-1 do
		local pos = positions[i]:ToScreen()
		local prevpos = positions[i+1]:ToScreen()
		surface.DrawRect(pos.x, pos.y, 4, 4)
		surface.DrawLine(pos.x, pos.y, prevpos.x,prevpos.y)
	end
end)

AddHook("CreateMove", function(cmd)
	local shiftingTickbase = samoware.GetShiftingTickbase()
	if settings.Aimbot.Silent and !shiftingTickbase then
		UpdateSilentAngles(cmd)
	end

	if cmd:CommandNumber() == 0 then return end

	-- Reset cl_interp
	if samoware.GetManipulateInterp() then
		samoware.SetTargetInterp(0)
	end

	if !shiftingTickbase then
		UpdateKeybinds()
	else
		samoware.AdjustTickbase()
	end

	bSendPacket = true
	_R.bSendPacket = bSendPacket

	if !me:Alive() then return end

	if !prevang then
		prevang = cmd:GetViewAngles()
	end

	if settings.HvH["Break lagcomp"] then
		SetBreakLagcompTicks()
	end

	if settings.Misc.AutoStrafe then AutoStrafe(cmd) end
	if settings.Misc.BunnyHop then BunnyHop(cmd) end

	if settings.HvH.Enginepred then samoware.StartPrediction(cmd) end

	positions[#positions+1] = me:GetPos()
	if #positions > 66 then
		table.remove(positions, 1)
	end

	if settings.HvH.FakeLag then
		FakeLag(cmd)
	else
		fakelagtick = 0
	end

	if settings.HvH["FakeLag in air"] then
		FakeLagInAir(cmd)
	end

	if settings.HvH["FakeLag on peek"] or settings.HvH["Warp on peek"] then
		CheckPeeking()
	end

	if settings.HvH.FakeDuck and keybinds.HvH.FakeDuckKey then
		FakeDuck(cmd)
	end

	-- Compute strafe simulations before aimbot
	StartCircleStrafe(cmd)

	local aimbotsucc = keybinds.Aimbot.Key and CanShoot(cmd) and Aimbot(cmd)
	if aimbotsucc then
		local angles = cmd:GetViewAngles()
		realangles = angles
		fakeangles = angles
	elseif !(cmd:KeyDown(IN_ATTACK) and CanShoot(cmd)) and !cmd:KeyDown(IN_USE) and !shiftingTickbase then
		if settings.HvH.Crimwalk and keybinds.HvH.CrimwalkKey then
			Crimwalk(cmd)
		else
			local freestandsucc, freestandsafe, freestandunsafe
			if settings.HvH.Freestand then
				freestandsucc, freestandsafe, freestandunsafe = Freestand(cmd)
			end

			local pitch, yaw
			if settings.HvH.AA != "None" and !freestandsucc then
				yaw, pitch = YawAntiAim(cmd)
			elseif freestandsucc then
				yaw = bSendPacket and freestandunsafe or freestandsafe
			end

			if settings.HvH.FakePitch != "None" then
				pitch = PitchAntiAim(cmd)
			end

			if settings.HvH.AA != "None" or settings.HvH.FakePitch != "None" or freestandsucc then
				local view = settings.Aimbot.Silent and silentangles or cmd:GetViewAngles()

				pitch = pitch or view.p
				-- yaw = mNormalizeAng(yaw or view.y)

				local angles = Angle(pitch, yaw, 0)
				cmd:SetViewAngles(angles)

				if bSendPacket then
					if settings.HvH.FakePitch == "Fake down" then
						angles.p = 89
					end

					fakeangles = angles
				else
					realangles = angles
				end
			else
				local angles = cmd:GetViewAngles()
				if settings.HvH.FakeLag then
					if bSendPacket then
						fakeangles = angles
					else
						realangles = angles
					end
				else
					realangles = angles
					fakeangles = angles
				end
			end
		end
	elseif cmd:KeyDown(IN_ATTACK) then
		if !CanShoot(cmd) then
			cmd:RemoveKey(IN_ATTACK)
		else
			local view = settings.Aimbot.Silent and silentangles or cmd:GetViewAngles()

			if settings.Aimbot.Norecoil then
				view = RemoveRecoil(view)
			end

			if settings.Aimbot.Nospread then
				view = RemoveSpread(cmd, view)
			end

			realangles = view
			fakeangles = view

			if !settings.HvH["FakeLag on shoot"] then
				bSendPacket = true
			end

			cmd:SetViewAngles(view)
		end
	else
		local view = cmd:GetViewAngles()

		realangles = view
		fakeangles = view
	end

	-------- KOSTIL
	if IsValid(me:GetActiveWeapon()) and me:GetActiveWeapon():GetClass():StartWith("m9k_") and cmd:KeyDown(IN_SPEED) and cmd:KeyDown(IN_ATTACK) then
		print("REMOVING IN_SPEED")
	end

	if settings.HvH.FakeEblan and !shiftingTickbase then
		FakeEblan(cmd)
	end

	if settings.HvH["Duck in air"] then
		DuckInAir(cmd)
	end

	if settings.HvH["Arm breaker"] and bSendPacket and !shiftingTickbase then
		if settings.HvH.ArmBreakerMode == "Full" then
			SetTyping(cmd)
		elseif settings.HvH.ArmBreakerMode == "Random" and mrandom(0, 1) == 0 then
			SetTyping(cmd)
		elseif settings.HvH.ArmBreakerMode == "Up/Down" and (cmd:CommandNumber() % 64) >= 32 then
			SetTyping(cmd)
		end
	end

	if settings.Misc["Flashlight spam"] then
		cmd:SetImpulse(100)
	end

	if settings.Misc.Fastwalk and mabs(cmd:GetSideMove()) < 1 and mabs(cmd:GetForwardMove()) > 1 then
		cmd:SetSideMove(cmd:CommandNumber() % 2 == 0 and -10000 or 10000)
	end

	if settings.Misc["Michael Jackson"] then
		MichaelJackson(cmd)
	end

	if settings.Aimbot.Silent then
		FixMove(cmd, silentangles)
	end

	-- Gather thread results
	FinishCircleStrafe(cmd)

	if settings.HvH.Enginepred then samoware.FinishPrediction() end

	if !shiftingTickbase then
		if settings.HvH.Warp and keybinds.HvH.WarpKey then
			ShiftTickbase(settings.HvH["Warp shift"])
		elseif settings.HvH.Doubletap and band(cmd:GetButtons(), IN_ATTACK) != 0 and CanShoot(cmd) then
			ShiftTickbase()
		end

		if keybinds.HvH.WarpRechargeKey then
			RechargeShift()
		end
	end

	if settings.Aimbot.AutoPistol and band(cmd:GetButtons(), IN_ATTACK) != 0 and CanShoot(cmd) then
		if autopistol then
			cmd:RemoveKey(IN_ATTACK)
		else
			cmd:SetButtons(bor(cmd:GetButtons(), IN_ATTACK))
		end

		autopistol = !autopistol
	end

	if settings.Misc["Spectator memes"] then
		local r
		local mode = settings.Misc.SpectatorMemeMode

		if mode == "Spin" then
			r = (CurTime() * 2.7) % 360
		elseif mode == "Sin" then
			r = msin(CurTime() * 6) * 180 + 180
		elseif mode == "Random" then
			r = mRand(0, 360)
		else
			r = 180
		end

		local view = cmd:GetViewAngles()
		view.r = r

		cmd:SetViewAngles(view)
	end

	if bSendPacket then
		chokes = 0
	else
		chokes = chokes + 1
	end

	if settings.Misc["SimplAC safe mode"] then
		local curang = cmd:GetViewAngles()
		local diff = curang - prevang
		prevang = curang

		cmd:SetMouseX(diff.y / -0.023)
		cmd:SetMouseY(diff.p / 0.023)
	end

	if !settings.Aimbot.Silent then
		silentangles = cmd:GetViewAngles()
	end

	if chokes >= 21 then print("CHOKES", chokes) end

	-- set original bSendPacket
	samoware.SetForceChoke(!bSendPacket)
end)

end

-- Utils
do
	concommand.Add("_sw_setname", function(_, _, _, name)
		name = name:Replace("\\n", "\n")
		name = name:Replace("\\t", "\t")

		print("Changing name to", name)

		samoware.NetSetConVar("name", name)
	end)

	concommand.Add("_sw_setname_long",function()
		samoware.NetSetConVar("name", "‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱‱")
	end)

	concommand.Add("_sw_startspam", function(_, _, _, text)
		text = text:Replace("\\n", "\n")
		text = text:Replace("\\t", "\t")

		hook.Add("Tick", "kek", function()
			RunConsoleCommand("say", text)
		end)
	end)

	concommand.Add("_sw_stopspam", function(_, _, _, text)
		hook.Remove("Tick", "kek")
	end)
end
