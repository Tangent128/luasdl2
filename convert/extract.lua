
local dumpPrefix = nil

for k, v in pairs{...} do
	local out = v:match "--out=(.+)"
	if out then
		dumpPrefix = out
	end
end

-- Reads the wiki dump HTML as stdin, tries to split it out into
-- the individual pages for later conversion to Markdown

local pages = {}
local currentPage = nil

for line in io.lines() do
	local hrMatch = line:match("<hr%s+/>")
	local aName = line:match([[<a.+name="([^"]+)".+>]])
	
	if hrMatch then
		currentPage = nil
	elseif aName then
		if currentPage == nil then
			currentPage = aName
			pages[currentPage] = pages[currentPage] or ""
		end
	--[=[elseif line:match[[href="#([^"]+)"]] then
		print(line)
		print(line:gsub([[href="#([^"]+)"]], [[href="%1"]]))
	]=]
	elseif currentPage ~= nil then
		pages[currentPage] = pages[currentPage] .. "\n" .. line
	end
	
end


-- fix cross-page link targets
local localLink = [[href="#%s"]]
local pageLink = [[href="%s"]]

for k, v in pairs(pages) do
	--print(#v, k)
	pages[k] = v:gsub([[href="#([^"]+)"]], function(name)
		if pages[name] then
			return(pageLink:format(name))
		else
			return(localLink:format(name))
		end
	end)
end

-- dump pages to individual files
if dumpPrefix then
	for k, v in pairs(pages) do
		local file = assert(io.open(dumpPrefix .. "/" .. k .. ".md", "w"))
		file:write(v)
		file:close()
	end
end
