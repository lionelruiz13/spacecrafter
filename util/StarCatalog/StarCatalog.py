#!/bin/python3
import threading

path = "/usr/local/share/spacecrafter/stars/"
endian = "little"

class StarArray:
	def __init__(self, starClass, file, level):
		self.zones = list()
		zoneStars = list()
		level = 20<<(level<<1)
		while level:
			zoneStars.append(int.from_bytes(file.read(4), endian))
			level -= 1
		for nbStars in zoneStars:
			self.zones.append(StarZone(starClass, file, nbStars))

	def findByCriteria(self, func):
		for z in enumerate(self.zones):
			for s in enumerate(z[1].stars):
				if func(s[1]):
					print("--> zones[", z[0], "].stars[", s[0], "] = ", s[1], sep="")
					return (z[0], s[0])

	def store(self, file):
		for zone in self.zones:
			file.write(len(zone.stars).to_bytes(4, endian))
		for zone in self.zones:
			zone.store(file)

class StarZone:
	def __init__(self, starClass, file, nbStars):
		self.stars = list()
		while nbStars:
			self.stars.append(starClass(file))
			nbStars -= 1
	def store(self, file):
		for star in self.stars:
			star.store(file)

class Star1:
	fields = (("hip", 3), ("component_ids", 1), ("x0", 4), ("x1", 4), ("b_v", 1), ("mag", 1), ("sp_int", 2), ("dx0", 4), ("dx1", 4), ("plx", 4))
	def __init__(self, file):
		for f in self.fields:
			setattr(self, f[0], int.from_bytes(file.read(f[1]), endian, signed=True))
	def store(self, file):
		for f in self.fields:
			file.write(getattr(self, f[0]).to_bytes(f[1], endian, signed=True))

class Star2:
	stride = 10
	fields = (("x0", 20), ("x1", 20), ("dx0", 14), ("dx1", 14), ("b_v", 7), ("mag", 5))
	def __init__(self, file):
		self.data = file.read(self.stride)
	def store(self, file):
		file.write(self.data)

class Star3:
	stride = 6
	fields = (("x0", 18), ("x1", 18), ("b_v", 7), ("mag", 5))
	def __init__(self, file):
		self.data = file.read(self.stride)
	def store(self, file):
		file.write(self.data)

class Catalog:
	fields = ("magic", "type", "major", "minor", "level", "mag_min", "mag_range", "mag_steps")
	def __init__(self, filename):
		print("Loading", filename)
		self.file = open(path+filename, "rb")
		self.filename = filename
		for f in self.fields:
			setattr(self, f, int.from_bytes(self.file.read(4), endian, signed=True))
		self.zone1 = StarArray(Star1, self.file, self.level)
		self.zone2 = StarArray(Star2, self.file, self.level)
		self.zone3 = StarArray(Star3, self.file, self.level)
		self.file.close()

	def __del__(self):
		self.store() # Save when leaving

	def store(self):
		file = open(self.filename, "wb", 664)
		for f in self.fields:
			file.write(getattr(self, f).to_bytes(4, endian, signed=True))
		self.zone1.store(file)
		self.zone2.store(file)
		self.zone3.store(file)
		file.close()

class Catalogs:
	catalogs = ("0v0_5", "0v0_5", "0v0_5")#, "1v0_3", "1v0_0", "2v0_0")#, "2v0_0", "2v0_0", "2v0_0"
	def __init__(self):
		self.cat = list()
	def load(self):
		for c in enumerate(self.catalogs):
			self.cat.append(Catalog("stars_" + str(c[0]) + "_" + c[1] + ".cat"))
		print("Loaded")
	def findByCriteria(self, zone, func):
		for c in self.cat:
			print("-----", c.filename, "-----")
			ret = getattr(c, "zone"+str(zone)).findByCriteria(func)

cat=Catalogs()
def asyncLoad():
	cat.load()
thread = threading.Thread(target=asyncLoad, name="Async Catalog Loader")
thread.start()

c = None
z = None
def select(catalog=0, zone=1):
	"""
	Select a catalog
	Catalog is catalog file index (or Catalog object)
	zone is star type (Star1, Star2, Star3)
	Star1 - Advanced star with hip number
	Star2 - Basic moving star
	Star3 - Basic static star
	"""
	global c, z
	if type(catalog) is int:
		c = cat.cat[catalog]
	else:
		c = catalog
	z = getattr(c, "zone"+str(zone))

star = None
def find(hip):
	global star
	zone, idx = z.findByCriteria(lambda s : s.hip == hip)
	star = z.zones[zone].stars[idx]
	return star

def pick(hip):
	global star
	zone, idx = z.findByCriteria(lambda s : s.hip == hip)
	star = z.zones[zone].stars.pop(idx)
	return star

def put(zone):
	global star
	z.zones[zone].stars.append(star)


# Run this code using "python3 -i StarCatalog.py"
# By default, catalog are read from source and written to the local folder (to prevent accidental overwrite)
# mycatalog = Catalog("stars_0_0v0_8.cat")
# select(mycatalog, 1) # Select mycatalog with star1 type (star with hip)
# find(10826) # Select star with hip 10826
# pick(10826) # Extract star with hip 10826 (removing it from the selected catalog)
# star.mag             (show the mag value of selected star in catalog range, real mag value is c.mag_min + c.mag_range * star.mag / 255)
# star.mag = 108       (set the scaled mag value of selected star to 108, which correspond to a mag of c.mag_min + c.mag_range * 108 / 255)
# put(65) # Put the selected star in the zone 65 of the currently selected catalog
