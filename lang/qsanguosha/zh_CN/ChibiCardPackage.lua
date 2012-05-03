-- translation for ChibiCardPackage

return {
	["ChibiCard"] = "赤壁",
	
	--basic
	["wind_slash"] = "风杀",
	[":wind_slash"] = "出牌时机：出牌阶段\
使用目标：除你外，你攻击范围内的一名角色\
作用效果：此【杀】造成伤害后，你须指定目标一张牌移出游戏，直到目标下个回合开始时返回其手牌。",
	["crisp"] = "酥",
	[":crisp"] = "出牌时机：出牌阶段\
使用目标：对自己使用\
作用效果：本回合内使用【桃】时可额外回复1点体力。（每回合限使用一次）",

	--equip	
	["catapult"] = "投石车",
	[":catapult"] = "攻击范围：5\
武器特效：当你对已装备【城墙】角色造成伤害时，你可令该伤害+1并弃置该角色的【城墙】。",
	["#CatapultDamage"] = "由于 %from 已装备【<font color='yellow'><b>城墙</b></font>】， %to 装备【<font color='yellow'><b>投石车</b></font>】的效果被触发，%from 受到的伤害由 %arg 增加到 %arg2",
	["bagua_sword"] = "八卦剑",
	[":bagua_sword"] = "攻击范围：2\
武器特效：你使用【杀】时，可进行一次判定，若结果不为红桃，你可为该【杀】指定一种属性，否则视为该【杀】已被目标闪避。",
	["#BaguaSwordLog"] = "%from 选择了 %arg ",
	["wall"] = "城墙",
	[":wall"] = "<b>锁定技，</b>任何属性杀及【水淹七军】对你无效；你受到的无属性伤害-1（至少为1）。",
	["#WallDamage"] = "%from 装备【<font color='yellow'><b>城墙</b></font>】的效果被触发，受到的伤害由 %arg 减少到 %arg2",	
	["black_armor"] = "玄甲",
	[":black_armor"] = "你即将受到伤害时，若你已受伤，你可以弃置X张红色手牌，则你即将受到的伤害转化为体力回复。（X为此次伤害的伤害量）",
	["@black_armor"] = "你即将受到伤害，你可以弃置X张红色手牌，将即将受到的伤害转化为体力回复。",
	["#BlackArmorSkill"] = "%from 发动了技能【<font color='yellow'><b>玄甲</b></font>】，受到的伤害转化为体力回复",
	["boat"] = "草船",
	[":boat"] = "<b>锁定技，</b>你受到的【火杀】伤害+1；你使用的【风杀】伤害+1。",
	["$BoatLog"] = "%from 装备【草船】的效果被触发，%card 造成的伤害 +1",
	
	--trick
	["sucker"] = "暗渡陈仓",
	[":sucker"] = "出牌阶段，对任意一名角色使用，将【暗度陈仓】横置于目标角色判定区里，目标角色回合判定阶段进行判定；若判定结果是黑色，则跳过目标角色的弃牌阶段，将【暗度陈仓】弃置",	
	["raid"] = "夜袭",
	[":raid"] = "出牌阶段对出自己以外任意一名角色使用，该角色需弃置1张黑色牌否则受到你对其造成的1点伤害。",	
	["@raid"] = "%src 使用了【夜袭】，请弃置一张黑色牌，否则 %src 将对你造成的1点伤害",
	["steal"] = "盗书",
	[":steal"] = "出牌阶段，对你攻击范围内的一名有手牌的角色使用，你声明一种花色，然后展示目标一张手牌，若与你声明的花色相同，则你对其造成一点伤害；否则你获得该牌。",
	["flooding"] = "水淹七军",
	[":flooding"] = "出牌阶段对其他角色使用，从牌堆顶亮出等同于现存角色数量的牌，弃掉其中的红色牌，按出牌顺序依次获得这些牌中的一张，获得牌的角色须弃掉一张红色牌，否则你对其造成一点伤害。",
	["@flooding"] = "%src 使用了【水淹七军】，请弃置一张红色牌，否则 %src 将对你造成的1点伤害",

	--common
	["windpile"] = "风牌",
	["@windpile"] = "风牌",
	["wind_nature"] = "风属性",
	["#Crisp"] = "%from 吃了【酥】，下一张【桃】将具有回复量 +1 的效果",
	["#CrispBuff"] = "%from 吃了【酥】，【桃】的回复量 +1",
	["#UnsetCrisp"] = "%from 桃的结算完毕，【酥】的效果消失",
	["#UnsetCrispEndOfTurn"] = "%from 的回合结束，【酥】的效果消失",
}