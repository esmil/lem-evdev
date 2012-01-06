#!/usr/bin/env lem
--
-- This file is part of lem-evdev.
-- Copyright 2011-2012 Emil Renner Berthing
--
-- lem-evdev is free software: you can redistribute it and/or
-- modify it under the terms of the GNU General Public License as
-- published by the Free Software Foundation, either version 3 of
-- the License, or (at your option) any later version.
--
-- lem-evdev is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with lem-evdev.  If not, see <http://www.gnu.org/licenses/>.
--

package.path = '?.lua;' .. package.path
package.cpath = '?.so;' .. package.cpath

local utils = require 'lem.utils'
local evdev = require 'lem.evdev'

local format = string.format

local function dumper(path, name)
	local dev = assert(evdev.open(path))

	while true do
		local typ, code, value, stamp = assert(dev:get())
		if not typ then error(code) end

		print(format('%s%-9s %4d %4d -- %s', name,
			typ, code, value, os.date('%c', stamp)))
	end
end

local function getname(n)
	local file = format('/sys/class/input/input%s/name', n)
	file = assert(io.open(file, 'r'))
	local name = assert(file:read('*a'))
	file:close()

	return name
end

for i = 1, #arg do
	local n = arg[i]
	local dev = format('/dev/input/event%s', n)
	local name = getname(n)
	print(name, dev)
	utils.spawn(dumper, dev, name)
end

-- vim: syntax=lua ts=2 sw=2 noet:
