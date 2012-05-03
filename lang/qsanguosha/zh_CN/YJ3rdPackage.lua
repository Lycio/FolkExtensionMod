-- translation for YJ3rd Package

return {
	["YJ3rd"] = "三将成名",
	
	["yjguohuai"] = "郭淮",
	["#yjguohuai"] = "挺身御蜀",
	["cv:yjguohuai"] = " ",
	["designer:yjguohuai"] = "小G",
	["illustrator:yjguohuai"] = " ",
	
	["yjjubing"] = "聚兵",
	[":yjjubing"] = "出牌阶段，你可以进行判定一次，若不为红桃，则你从当前弃牌堆中选取一张装备牌将其收为手牌（弃牌堆中无装备牌时仅判定而不能获得任何卡牌）",
	["yjzhufang"] = "筑防",
	[":yjzhufang"] = "<b>觉醒技，</b>回合开始阶段开始时，若你装备区有两张或以上牌，你须减少1点体力上限，回复1点体力或摸两张牌，并获得技能“诱进”（出牌阶段，你可交给一名其他角色一张【杀】或装备牌，则在你回合结束时该角色须执行一个出牌阶段；若其在此阶段未使用基本牌，则其武将牌翻面。每阶段限一次）",
	["yjyoujin"] = "诱进",
	[":yjyoujin"] = "出牌阶段，你可交给一名其他角色一张【杀】或装备牌，则在你回合结束时该角色须执行一个出牌阶段；若其在此阶段未使用基本牌，则其武将牌翻面（每阶段限一次）",
	["#YjZhufangWake"] = "%from 的觉醒技【%arg】被触发",
	["#YjYoujin"] = "%from 将进行额外的 <font color='yellow'><b>出牌</b></font> 阶段",
	
	["yjkongrong"] = "孔融",
	["#yjkongrong"] = "恃才负气",
	["cv:yjkongrong"] = " ",
	["designer:yjkongrong"] = "这只是个马甲",
	["illustrator:yjkongrong"] = " ",
	
	["yjrangli"] = "让梨",
	[":yjrangli"] = "出牌阶段，若你已受伤，你可以摸X张牌后展示所有手牌，将其中点数较大的一半（向上取整）交给其他一名角色，若给出三张或更多的牌，你回复1点体力。每阶段限一次。X为你已损失的体力值",
	["yjcibian"] = "辞辩",
	[":yjcibian"] = "每当你受到一次伤害，你可令伤害来源展示一张手牌后展示牌堆顶的一张牌（进行拼点），若该角色没赢，你回复1点体力",
	["$YjCibian_show"] = "%from 展示了牌堆顶的牌 %card ",
	["#YjCibian"] = "%from 与 %arg 进行拼点 %arg2 ",
	["DrawPile"] = "牌堆",
	["win"] = "赢了",
	["lose"] = "输了",

	["yjcaorui"] = "曹睿",
	["#yjcaorui"] = "明主一时",
	["cv:yjcaorui"] = " ",
	["designer:yjcaorui"] = "qiushanliao",
	["illustrator:yjcaorui"] = " ",
	
	["yjmingmin"] = "明敏",
	[":yjmingmin"] = "每当你受到一次伤害后，可进行一次判定，然后打出一张手牌，若与判定牌同颜色，你摸一张牌；若与判定牌同花色，你弃置来源的一张牌",
	["yjpianquan"] = "偏权",
	[":yjpianquan"] = "任意角色进入判定阶段时，若其判定区不为空，你可令另一名其他角色摸一张牌",
	["yjrenxin"] = "任心",
	[":yjrenxin"] = "<b>主公技，</b>每当其他魏势力角色体力减少时，可令你选择一项执行：摸一张牌或获得其所在处的一张牌",
	["@yjmingmin"] = "判定结果为 %arg 请打出一张 %2arg 手牌响应技能【明敏】",
	["yjrenxin:getacard"] = "获得该角色一张牌",
	
	["yjmaliang"] = "马良",
	["#yjmaliang"] = "五常白眉",
	["cv:yjmaliang"] = " ",
	["designer:yjmaliang"] = "孔明灵格斯|qiushanliao|这只是个马甲",
	["illustrator:yjmaliang"] = " ",
	
	["yjnaman"] = "纳蛮",
	[":yjnaman"] = "摸牌阶段摸牌前，若你已受伤，你可以展示牌堆顶的X张牌，然后将这些牌置于一旁称为“蛮”；你可将任意角色响应【南蛮入侵】时打出的牌加为“蛮”（X为你已损失的体力值；“蛮”至多为5张）",
	["yjzhiman"] = "治蛮",
	[":yjzhiman"] = "<b>锁定技，</b>你不能成为【南蛮入侵】的目标",
	["yjyinjian"] = "引荐",
	[":yjyinjian"] = "出牌阶段，你可以弃掉一张“蛮”，然后执行下列两项中的一项：1.令攻击范围内的一名角色摸一张牌；2.视为对攻击范围外的一名角色使用了一张【杀】，每阶段限用一次",
	["savage"] = "蛮",
	
	["yjkanze"] = "阚泽",
	["#yjkanze"] = "儒林一时",
	["cv:yjkanze"] = " ",
	["designer:yjkanze"] = "qiushanliao",
	["illustrator:yjkanze"] = " ",
	
	["yjzhuji"] = "助计",
	[":yjzhuji"] = "任意角色的出牌阶段，若其失去的手牌达到三张时，你可以立即执行以下两项中的一项：弃置其一张手牌；令其摸一张牌（每阶段限一次）",
	["yjzhuji:to_discard"] = "弃掉其一张手牌",
	["yjzhuji:to_draw"] = "令其摸一张牌",
	["yjguzong"] = "故纵",
	[":yjguzong"] = "出牌阶段，你可以展示一张手牌后指定其他一名角色，该角色须执行以下两项中的一项：\
	1.获得此牌，若在其回合内弃牌阶段弃牌前，此牌未进入弃牌堆，你对此牌拥有者造成1点伤害；\
	2.弃置一张与此牌相同的牌，令你流失1点体力并摸一张牌。\
	（每阶段限一次）\
	★“相同的牌”指牌面、花色、点数均一致",
	["accept_to_gain"] = "获得此牌",
	["refuse_to_gain"] = "放弃此牌",
	["#YjGuzongSelect"] = "%from 选择了 %arg",
	["#YjGuzongTrigger"] = "%from 的技能【%arg2】被触发，将对 %to 造成 %arg 点伤害",
	
	["yjzhoucang"] = "周仓",
	["#yjzhoucang"] = "一生侍侯",
	["cv:yjzhoucang"] = " ",
	["designer:yjzhoucang"] = "sands",
	["illustrator:yjzhoucang"] = " ",
	
	["yjshiwu"] = "侍武",
	[":yjshiwu"] = "其他角色的杀被闪避时，你可弃掉一张非基本牌令杀的来源再使用一张杀（不能指定本阶段内已成为杀的目标的角色），若其不如此做，你弃置杀的来源2张牌（不足2张则失去1点体力），每阶段限一次",
	["@yjshiwu"] = "你要发动技能【%arg】吗？",
	["@yjshiwu-slash"] = "%src要求你使用一张【杀】",
	
	["$ShowACard"] = "%from 展示了 %card ",
	["$ResponseCard"] = "%from 打出了 %card ",
	["$ToDiscardCard"] = "%from 弃置了 %to 的 %card ",
}
