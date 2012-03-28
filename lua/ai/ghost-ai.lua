sgs.ai_skill_invoke.xiaoshou = function(self, data)
	local source = data:toPlayer()
	return self:isEnemy(source)
end

