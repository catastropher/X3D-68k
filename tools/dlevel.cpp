#include <iostream>
#include <string>
#include <iostream>
#include <cstdio>
#include <vector>
#include <cstdlib>

using namespace std;

unsigned int file_read_int(FILE *f) {
	unsigned int n = 0;
	
	for(int i = 0;i < 4;i++) {
		n = (n >> 8) | ((int)fgetc(f) << 24);
	}
	
	return n;
}

unsigned short file_read_short(FILE *f) {
	unsigned int n = 0;
	
	for(int i = 0;i < 2;i++) {
		n = (n >> 8) | ((int)fgetc(f) << 8);
	}
	
	return n;
}

struct HogFile {
	string name;
	vector<unsigned char> data;
};

struct Vex3D {
	int x, y, z;
	
	Vex3D(int xx, int yy, int zz) : x(xx), y(yy), z(zz) { }
	
	Vex3D() { }
};

struct Cube {
	Vex3D v[8];
	int cube[6];
};

struct Level {
	vector<Cube> cubes;

	bool loadFromHog(HogFile &f);
	void save(string file);
	bool convertRegistered(HogFile& f, HogFile& dest, string name);
};

struct Reader {
	vector<unsigned char> *data;
	int pos;
	
	Reader(vector<unsigned char> *d, int p) : data(d), pos(p) {}
	
	int getc() {
		if(pos < data->size())
			return (*data)[pos++];
		else
			return EOF;
	}
	
	unsigned char readByte() {
		return getc();
	}
	
	bool eof() {
		return pos < data->size();
	}
	
	unsigned int readInt() {
		unsigned int n = 0;
		
		for(int i = 0;i < 4;i++) {
			n = (n >> 8) | ((unsigned int)getc() << 24);
		}
		
		return n;
	}
	
	unsigned short readShort() {
		unsigned short n = 0;
		
		for(int i = 0;i < 2;i++) {
			n = (n >> 8) | ((int)getc() << 8);
		}
		
		return n;
	}
	
	
};

struct VectorWriter {
	vector<unsigned char>* data;
	
	void writeByte(unsigned char b) {
		data->push_back(b);
	}
	
	void writeShort(unsigned short s) {
		writeByte(s & 0xFF);
		writeByte(s >> 8);
	}
	
	void writeInt(unsigned int i) {
		writeShort(i & 0xFFFF);
		writeShort(i >> 16);
	}
};

bool Level::convertRegistered(HogFile& f, HogFile& dest, string name) {
	Reader r(&f.data, 0);
	VectorWriter w;
	w.data = &dest.data;
	
	if(f.data[0] != 'L' || f.data[1] != 'V' || f.data[2] != 'L' || f.data[3] != 'P') {
		cout << "Error: file is not a Descent 1 Level" << endl;
		return false;
	}
	
	r.pos = 4;
	
	cout << "File version: " << (int)r.readInt() << endl;
	
	int mine_offset = r.readInt();
	
	printf("Offset: %d\n", mine_offset);
	
	int object_offset = r.readInt();
	int file_size = r.readInt();
	
	r.pos = mine_offset;
	
	unsigned char version = r.readByte();
	unsigned short total_v = r.readShort();
	
	// Number of cubes
	unsigned short total_c = r.readShort();
	
	vector<Vex3D> v;
	
	
	
	// Read in the vertices
	for(int i = 0; i < total_v; i++) {
		int x = r.readInt();
		int y = r.readInt();
		int z = r.readInt();
		
		v.push_back(Vex3D(x, y, z));
	}
	
	w.writeByte('L');
	w.writeByte('V');
	w.writeByte('L');
	w.writeByte('P');
	
	// File version
	w.writeInt(0);
	
	// Mine offset (TODO: determine)
	w.writeInt(w.data->size() + 8 + 4);
	
	// Object offset
	w.writeInt(0);
	
	// File size
	w.writeInt(0);
	
	// Mine version
	w.writeByte(0);
	
	// Total vertices
	w.writeInt(total_v);
	
	// Total cubes
	printf("TOTAL CUBES: %d\n", total_c);
	
	w.writeInt(total_c);
	
	// Write the vertices
	for(int i = 0; i < total_v; i++) {
		w.writeInt(v[i].x);
		w.writeInt(v[i].y);
		w.writeInt(v[i].z);
	}
	
	// Write the cubes
	for(int i = 0; i < total_c; i++) {
		// Mask describing which cubes have a side attached
		unsigned char mask = r.readByte();
		short children[6];
		
		
		// Read each child, if there is one
		for(int d = 0; d < 6; d++) {
			if(mask & (1 << d)) {
				children[d] = r.readShort();
				w.writeShort(children[d]);
				
			}
			else {
				// No child...
				w.writeShort(-1);
				children[d] = -1;
			}
		}
		
		// Vertex id's
		for(int d = 0; d < 8; d++) {
			short v = r.readShort();
			
			//printf("Vertex id: %d\n", v);
			//system("read");
			
			w.writeShort(v);
		}
		
		unsigned char special;
		char matcen;
		unsigned short value;
		
		if(mask & (1 << 6)) {
			// We don't use these values
			special = r.readByte();
			matcen = r.readByte();
			value = r.readShort();
		}
		else {
			special = 0;
			matcen = -1;
			value = 0;
		}
		
		w.writeByte(special);
		w.writeByte(matcen);
		w.writeShort(value);
		
		unsigned short light = r.readShort();
		
		w.writeShort(light);
		
		// Wall bitmask
		unsigned char wall_mask = r.readByte();
		
		short wall_num[6];
		unsigned char wall_num_byte[6];
		
		for(int d = 0; d < 6; d++) {
			if(wall_mask & (1 << d)) {
				wall_num_byte[d] = r.readByte();
				
				if(wall_num_byte[d] == 255)
					wall_num[d] = -1;
				else
					wall_num[d] = wall_num_byte[d];
				
			}
			else {
				wall_num[d] = -1;
				wall_num_byte[d] = 255;
			}
			
			// Save the wall number
			w.writeByte(wall_num_byte[d]);
		}
		
		for(int d = 0; d < 6; d++) {
			if(children[d] == -1 || wall_num[d] != -1) {
				unsigned short temp_ushort = r.readShort();
				
				// tmap
				w.writeShort(temp_ushort);
				
				// tmap2
				if(temp_ushort & 0x8000) {
					w.writeShort(r.readShort());
				}
				else {
					w.writeShort(0);
				}
				
				// uvl values
				for(int k = 0; k < 4; k++) {
					w.writeShort(r.readShort());
					w.writeShort(r.readShort());
					w.writeShort(r.readShort());
				}
			}
		}
	}
	
	return true;
}

bool Level::loadFromHog(HogFile &f) {
	Reader r(&f.data, 0);
	
	if(f.data[0] != 'L' || f.data[1] != 'V' || f.data[2] != 'L' || f.data[3] != 'P') {
		cout << "Error: file is not a Descent 1 Level" << endl;
		return false;
	}
	
	r.pos = 4;
	
	cout << "File version: " << (int)r.readInt() << endl;
	
	int mine_offset = r.readInt();
	int object_offset = r.readInt();
	int file_size = r.readInt();
	
	cout << "Mine offset: " << mine_offset << endl;
	cout << "Object offset: " << object_offset << endl;
	cout << "File size: " << file_size << endl << endl;
	
	printf("Real size: %d\n", r.data->size());
	
	cout << "Reading mine..." << endl;
	r.pos = mine_offset;
	
	cout << "Mine version: " << (int)r.getc() << endl;
	
	unsigned short total_v = r.readInt();
	unsigned short total_cubes = r.readInt();
	
	cout << "Total vertices: " << total_v << endl;
	cout << "Total cubes: " << total_cubes << endl;
	
	vector<Vex3D> v;
	
	// Read in the vertices
	for(int i = 0; i < total_v; i++) {
		int x = r.readInt();
		int y = r.readInt();
		int z = r.readInt();
		
		//printf("Vex3D: {%f, %f, %f}\n", x / 65536.0, y / 65536.0, z / 65536.0);
		
		//system("read");
		
		v.push_back(Vex3D(x, y, z));
	}
	
	short children[6];
	short cube_v[8];
	short wall_num[6];
	
	Cube c;
	
	for(int i = 0; i < total_cubes; i++) {
		// Read in the children segment ID's
		for(int d = 0; d < 6; d++) {
			children[d] = r.readShort();
			
			c.cube[d] = children[d];
			
			//printf("Children %d: %d\n", d, children[d]);
			//system("read");
		}
		
		// Read in the vertices that make up the cube
		for(int d = 0; d < 8; d++) {
			cube_v[d] = r.readShort();
			
			c.v[d] = v[cube_v[d]];
			
			//cout << "V: " << cube_v[d] << endl;
			
			//cout << "X: " << v[cube_v[d]].x / 65536.0 << "Y: " << v[cube_v[d]].y / 65536.0 << "Z: " << v[cube_v[d]].z / 65536.0 << endl;
			
			//system("pause");
		}
		
		unsigned char special = r.readByte();		// Skip special byte
		
		/////printf("Special: %d\n", special);
		
		r.readByte();		// Skip matcen_num
		r.readShort();		// Skip value
		
		r.readShort();		// Skip light value
		
		// Skip the 6 walls
		for(int d = 0; d < 6; d++) {
			wall_num[d] = r.readByte();
			
			if(wall_num[d] == 255) {
				wall_num[d] = -1;
				//printf("IS NEG 1\n");
			}
			
			//printf("Wall %d: %d\n", d, wall_num[d]);
		}
	
		for(int d = 0; d < 6; d++) {
			if(children[d] == -1 || wall_num[d] != -1) {
				r.readShort();	// Skip tmap_num
				r.readShort();	// Skip tmap_num2
				
				for(int k = 0; k < 4; k++) {
					r.readShort();		// Skip uvl values
					r.readShort();
					r.readShort();
				}
			}
		}
		
		cubes.push_back(c);
		
		
		
		//system("read");
	}
	
	return true;
}

struct Writer {
	FILE* file;
	
	void writeByte(unsigned char b) {
		fputc(b, file);
	}
	
	void writeShort(unsigned short s) {
		writeByte(s >> 8);
		writeByte(s & 0xFF);
	}
	
	void writeInt(unsigned int i) {
		writeShort(i >> 16);
		writeShort(i & 0xFFFF);
	}
};


void Level::save(string file_name) {
	Writer w;
	
	w.file = fopen(file_name.c_str(), "wb");

	unsigned short size = cubes.size();
	
	// Total number of cubes
	w.writeByte('X');
	w.writeByte('3');
	w.writeByte('D');
	w.writeByte('X');
	w.writeByte('D');
	w.writeByte('L');
	
	// Version info
	w.writeByte(0);
	w.writeShort(size);

	for(int i = 0; i < size; i++) {
		// Write the vertices
		for(int d = 0; d < 8; d++) {
			Vex3D v = cubes[i].v[d];
			
			short x = v.x >>= 16;
			short y = v.y >>= 16;
			short z = v.z >>= 16;
			
			w.writeShort(x);
			w.writeShort(y);
			w.writeShort(z);
		}
		
		// Write the neighbors
		for(int d = 0; d < 6; d++) {
			//fwrite(&cubes[i].cube[i], sizeof(short), 1, file);
			w.writeShort(cubes[i].cube[d]);
		}
	}
	
	fclose(w.file);
}



struct Hog {
	vector<HogFile *> files;
	bool load(string name);
	
	~Hog() {
		for(auto i : files)
			delete i;
	}
};

bool Hog::load(string name) {
	FILE *f = fopen(name.c_str(), "rb");
	
	if(!f) {
		cout << "Error opening file" << endl;
		return false;
	}
	
	if(fgetc(f) != 'D' || fgetc(f) != 'H' || fgetc(f) != 'F') {
		cout << "Error: file is not a HOG file" << endl;
		fclose(f);
		return false;
	}
	
	bool done = false;
	
	do {
		bool add = true;
		string name;
		
		for(int i = 0;i < 13;i++) {
			int c = fgetc(f);
			
			if(feof(f)) {
				done = true;
				break;
			}
			
			if(c == 0)
				add = false;
			
			if(add)
				name += c;
		}
		
		if(done)
			break;
		
		int size = file_read_int(f);
		
		HogFile *hf = new HogFile;
		
		hf->name = name;
		
		for(int i = 0;i < size;i++) {
			hf->data.push_back(fgetc(f));
		}
		
		cout << "Loaded file '" << name << "' (" << size << " bytes)" << endl;
		
		files.push_back(hf);
	} while(1);
			
	fclose(f);
	return true;
}

#include <cstdlib>

int main() {
	string file;
	cout << "HOG File to load: ";
	getline(cin, file);
	
	vector<Cube> cubes;
	
	Hog hog;
	if(!hog.load(file)) {
		return 0;
	}
	
	system("rmdir files2");
	system("mkdir files2");
	//system("cd files");
	
	cout << "Loaded HOG file" << endl << endl;
	
	for(auto &i : hog.files) {
		FILE *f = fopen(("files2/" + i->name).c_str(), "wb");
		
		fwrite((void *)&i->data[0], 1, i->data.size(), f);
		
		fclose(f);
	}
	
	Level level;
	
	bool found_level = false;
	
	string name = "level01";
	
	for(auto i : hog.files) {
		if(i->name == name + ".sdl") {
			if(!level.loadFromHog(*i)) {
				return 0;
			}
			
			cout << "Loaded Descent 1 level" << endl;
			found_level = true;
			break;
		}
		else if(i->name == name + ".rdl") {
			HogFile converted;
			
			if(!level.convertRegistered(*i, converted, name))
				return 0;
			
			if(!level.loadFromHog(converted)) {
				return 0;
			}
		
			cout << "Successfully loaded registered Descent 1 level" << endl;
			
			found_level = true;
			break;
		}
	}
	
	if(!found_level) {
		cout << "Error: level not found" << endl;
		return 0;
	}

	level.save(name + ".xdl");
	cout << "File saved to: " << name << ".xdl" << endl;
	
	cout << "Wrapping in calc variable..." << endl;
	
	system(("./tovar -92 XDL " + name + ".xdl " + name).c_str());
	
	
	return 0;
}