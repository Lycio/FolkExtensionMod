-- translation for NostalgiaPackage

return {
	["huangjin"] = "黄巾之乱",

	--张梁
	["zhangliang"] = "张梁",
	["rengong"] = "人公",
	[":rengong"] = "摸牌阶段，你可以选择以下行动来取代摸牌：观看任意一名角色的手牌一次，然后选择其中一张收入手牌。",
	["shishi"] = "噬尸",
	[":shishi"] = "每当你杀死任意一名角色后，可令自己的体力回复满。",
	["@rengong"] = "请指定【人公】的目标",

	--张燕
	["zhangyan"] = "张燕",
	["feiyan"] = "飞燕",
	[":feiyan"] = "你的回合外，每当你的攻击范围内的其它角色使用【杀】时，你可以立即对其使用一张杀，若如此做，你立即摸一张牌。",
	["@feiyan-slash"] = "%src 使用了一张【杀】，请对其使用一张【杀】。",
	["#FeiyanNoSlash"] = "%from 放弃对 %to 使用<font color='yellow'><b>【杀】</b></font>，<font color='yellow'><b>【飞燕】</b></font>发动失败",
	
	--张宝
	["zhangbao"] = "张宝",
	["zhenhuo"] = "真火",
	[":zhenhuo"] = "每当你使用的【杀】被【闪】抵消时，可令目标角色判定，若为【红桃】，你对该角色攻击范围内的任意一名角色造成1点火焰伤害。\
	◆可以指定目标角色本身为【真火】目标。",
	["heiyan"] = "黑烟",
	[":heiyan"] = "<b>锁定技</b>，你不能成为黑色的【杀】或【决斗】的目标。\
	◆与你【决斗】的角色依然可以打出黑色的【杀】。",
	["guishu"] = "鬼术",
	[":guishu"] = "在任意一名角色的判定牌生效前，你可以用自己的一张红色牌替换之。",
	["@guishu-card"] = "请使用【鬼术】修改 %src 的 %arg 判定",
	["@zhenhuo"] = "请指定【真火】的目标",

	--张曼成 4血
	["zhangmancheng"] = "张曼成",
	["weicheng"] = "围城",
	[":weicheng"] = "出牌阶段，你可以与你手牌数不同的一名角色拼点：\
	若你赢，你可以令其跳过其下回合的摸牌阶段或出牌阶段；\
	若你没赢，你获得以下效果直到你下回合开始：每当你成为【杀】或【决斗】的目标时，须立刻弃一张装备区内的牌；\
	每回合限一次。",
	["skipdraw"] = "令其跳过摸牌阶段",
	["skipplay"] = "令其跳过出牌阶段",
	["#WeichengSkipdraw"] = "%from 成功<font color='yellow'><b>【围城】</b></font>，%to 将跳过 <font color='yellow'><b>摸牌阶段</b></font>",
	["#WeichengSkipplay"] = "%from 成功<font color='yellow'><b>【围城】</b></font>，%to 将跳过 <font color='yellow'><b>出牌阶段</b></font>",
	["#WeichengClear"] = "%from 的<font color='yellow'><b>【围城】</b></font>效果已经消失",
	["#WeichengFailed"] = "%from 对 %to 发动<font color='yellow'><b>【围城】</b></font>失败",
	["#WeichengDiscard"] = "%from 受到<font color='yellow'><b>【围城】</b></font>失败的影响，弃掉了一张装备牌",

	--程远志 4血
	["chengyuanzhi"] = "程远志",
	["huangjin_baonue"] = "暴虐",
	[":huangjin_baonue"] = "当你造成伤害时，若目标角色已受伤，你可令其判定，若为【梅花】，则该伤害+1。",
	["#HuangjinBaonueEffect"] = "%from 的<font color='yellow'><b>【暴虐】</b></font>效果被触发，%to 受到的伤害从 %arg 点增加到 %arg2 点",

	--管亥 4血
	["guanhai"] = "管亥",
	["jieliang"] = "借粮",
	[":jieliang"] = "当你使用的【杀】造成伤害时，你可以防止此伤害，改为获取目标角色X+1张牌，X为此伤害的点数。\
	◆能增加【杀】造成伤害的情况包括酒、神力和裸衣三种，藤甲、狂风、古锭刀等属于发动时机冲突或发动时机未到，因此不计入【借粮】。",

	--裴元绍 4血
	["peiyuanshao"] = "裴元绍",
	["duoma"] = "夺马",
	[":duoma"] = "摸牌阶段，你可以选择少摸一张牌，若如此做，你立即获得其他角色装备区里的一张牌。\
	◆若其他所有角色的装备区内都没有牌，你便不能发动【夺马】。",
	["@duoma"] = "请指定【夺马】的目标",

	--周仓 4血
	["zhoucang"] = "周仓",
	["pengdao"] = "捧刀",
	[":pengdao"] = "每当你使用的【杀】被【闪】抵消时，你可以立即对此目标再使用一张【杀】；当你使用的【杀】造成伤害时，该伤害+X，X为你发动【捧刀】使用【杀】的次数。",
	["chixie"] = "持械",
	[":chixie"] = "<b>锁定技</b>，当你没装备防具时，你的手牌上限+1。",
	["@pengdao-slash"] = "你可以发动【捧刀】的追杀效果再对当前目标使用一张【杀】",
	["#PengdaoMiss"] = "%from 放弃发动<font color='yellow'><b>【捧刀】</b></font>，追杀 %to 失败",
	["#PengdaoEffect"] = "%from 的<font color='yellow'><b>【捧刀】</b></font>效果被触发，%to 受到的伤害从 %arg 点增加到 %arg2 点",
	
	--张角
	["huangjinzhangjiao"] = "张角",
	
	--马元义
	["mayuanyi"] = "马元义",
	["leichui"] = "雷锤",
	[":leichui"] = "当你使用的【杀】造成伤害后，可立即令目标判定，若结果为黑色，则目标额外受到1点雷电伤害。\
	◆技能发动为插入结算，当目标处于横置状态时，即使此技能由属性【杀】触发，也只能在判定结束后再结算【铁锁连环】，若判定成功，则传导1点雷伤害，否则传导原属性伤害。",
	
	--仙·于吉
	["xianyuji"] = "仙·于吉",
	["mabi"] = "麻痹",
	[":mabi"] = "当你使用的【杀】造成伤害时，你可以防止此伤害，若如此做，目标角色失去下回合摸牌阶段。",
	["xiuzhen"] = "修真",
	[":xiuzhen"] = "每当你受到1点伤害，可令伤害来源进行一次判定，判定结果为：\
	黑桃：该角色受到1点雷电伤害；\
	红桃：你回复1点体力；\
	梅花：该角色弃两张牌；\
	方块：你立即摸两张牌。\
	◆判定结果为黑桃时，此雷电伤害的来源为你自己。",
	["#MabiSkipdraw"] = "%to 受到 %from 的<font color='yellow'><b>【麻痹】</b></font>影响，将跳过下回合的 <font color='yellow'><b>摸牌阶段</b></font>",
	["#MabiClear"] = "%from 的<font color='yellow'><b>【麻痹】</b></font>效果已经消失",

	--仙·南华老仙
	["xiannanhua"] = "仙·南华",
	["huoqi"] = "祸起",
	[":huoqi"] = "若你于弃牌阶段弃掉了两张或更多的手牌，你可以在回合结束阶段执行下列两项中的一项：\
	1、回复1点体力；\
	2、指定任意两名合理的角色拼点，输的一方须弃两张牌或受到另一方造成的1点伤害。\
	◆“合理的角色”意味着你不能指定无手牌角色，若有手牌角色不足二人则不能发动祸起拼点。\
	◆发动【祸起】拼点时须指定两次目标，每次一人，视为由第一个目标角色向第二个目标角色发起拼点；你可以指定自己为拼点角色之一。",
	["yuli"] = "渔利",
	[":yuli"] = "每当一次拼点结束后，你可以立即摸一张牌。",
	["tianbian"] = "天变",
	[":tianbian"] = "在任意角色的拼点牌展示前，你可以用一张手牌替换之，每次拼点过程中限换一次。",
	["@huoqi"] = "请选取一位【祸起】拼点目标（共需选择两次，每次一位）。",
	["huoqirecover"] = "回复一点体力",
	["huoqipindian"] = "发动祸起拼点",
	["huoqinothing"] = "什么事都不做",
	["@tianbian-card"] = "请打出一张手牌来替换此次拼点中某一角色的拼点牌。",

	--武将台词
	
	--武将阵亡台词
	["~zhangliang"] = "",
	["~zhangyan"] = "",
	["~zhangbao"] = "",
	["~zhangmancheng"] = "",
	["~chengyuanzhi"] = "",
	["~guanhai"] = "",
	["~peiyuanshao"] = "",
	["~zhoucang"] = "",
	["~mayuanyi"] = "",
	["~xianyuji"] = "",
	["~xiannanhua"] = "",
	
	["cv:zhangliang"] = "",
	["cv:zhangyan"] = "",
	["cv:zhangbao"] = "",
	["cv:zhangmancheng"] = "",
	["cv:chengyuanzhi"] = "",
	["cv:guanhai"] = "",
	["cv:peiyuanshao"] = "",
	["cv:zhoucang"] = "",
	["cv:mayuanyi"] = "",
	["cv:xianyuji"] = "",
	["cv:xiannanhua"] = "",
	
	["designer:zhangliang"] = "杀神附体设计",
	["designer:zhangyan"] = "杀神附体设计",
	["designer:zhangbao"] = "杀神附体设计 Alcatraz修正",
	["designer:zhangmancheng"] = "杀神附体设计  Alcatraz修正",
	["designer:chengyuanzhi"] = "杀神附体设计  Alcatraz修正",
	["designer:guanhai"] = "杀神附体设计",
	["designer:peiyuanshao"] = "杀神附体设计",
	["designer:zhoucang"] = "杀神附体设计  Alcatraz修正",
	["designer:mayuanyi"] = "杀神附体设计",
	["designer:xianyuji"] = "杀神附体设计  Alcatraz修正",
	["designer:xiannanhua"] = "杀神附体设计",
	
	["title:zhangliang"] = "人公将军",
	["title:zhangyan"] = "燕山帅",
	["title:zhangbao"] = "地公将军",
	["title:zhangmancheng"] = "角阳寇",
	["title:chengyuanzhi"] = "短命鬼",
	["title:guanhai"] = "暴乱的贼寇",
	["title:peiyuanshao"] = "盗马贼",
	["title:zhoucang"] = "贴身侍卫",
	["title:mayuanyi"] = "大方首领",
	["title:xianyuji"] = "太平青领道",
	["title:xiannanhua"] = "乱世罪魁",
	
	["code:zhangliang"] = "Alcatraz",
	["code:zhangyan"] = "Alcatraz",
	["code:zhangbao"] = "Alcatraz",
	["code:zhangmancheng"] = "Alcatraz",
	["code:chengyuanzhi"] = "Alcatraz",
	["code:guanhai"] = "Alcatraz",
	["code:peiyuanshao"] = "Alcatraz",
	["code:zhoucang"] = "Alcatraz",
	["code:mayuanyi"] = "Alcatraz",
	["code:xianyuji"] = "Alcatraz",
	["code:xiannanhua"] = "Alcatraz",
}
