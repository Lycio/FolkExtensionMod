
-- this script to store the basic configuration for game program itself
-- and it is a little different from config.ini

config = {
	version = "20120405",
	version_name = "踏青版",
	mod_name = "FolkExtensionMod_ver_0.49",
	kingdoms = { "wei", "shu", "wu", "qun", "yan", "god"},
	
	package_names = {
		"StandardCard",
        "StandardExCard",
        "Maneuvering",
        "SPCard",
        "YitianCard",
        "Nostalgia",
        "Joy",
        "Disaster",
        "JoyEquip",
		"DishaCard",
        "QHSEquip",
        "Hide",

        "Standard",
        "Wind",
        "Fire",
        "Thicket",
        "Mountain",
        "God",
        "SP",
        "YJCM",
        "YJCM2012",
        "Special3v3",
        "BGM",
		"NostalGeneral",
        "Yitian",
        "Wisdom",
		"Huangjin",
        "Ghost",
        "QHS",
        "YJ1st",
        "YJ3rd",
        "TBdiy",
        "Yan",
        "ChangbanSlope",
        "Test",
	},

	scene_names = {
		"Guandu",
        "Fancheng",
        "Couple",
        "Zombie",
        "Impasse",
        "Custom",
	},
	
	color_wei = "#547998",
	color_shu = "#D0796C",
	color_wu = "#4DB873",
	color_qun = "#8A807A",
	color_yan = "#77549A",
	color_god = "#96943D",
 }
 
for i=1, 21 do
	local scene_name = ("MiniScene_%02d"):format(i)
	table.insert(config.scene_names, scene_name)
end