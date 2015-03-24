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

#if 0
bool Level::loadFromHog(HogFile &f) {
	cout << "Loading level from: " << f.name << endl;
	
	if(f.data[0] != 'L' || f.data[1] != 'V' || f.data[2] != 'L' || f.data[3] != 'P') {
		cout << "Error: file is not a Descent 1 Level" << endl;
		return false;
	}
	
	Reader r(&f.data, 0);
	
	for(int i = 0;i < f.data.size();i++) {
		//cout << i << "\t" << (int)f.data[i] << endl;
	}
	
	r.pos = 4;
	
	cout << "Version: " << (int)r.readInt() << endl;
	
	int mine_offset = r.readInt();
	int object_offset = r.readInt();
	int file_size = r.readInt();
	
	cout << "Mine offset: " << mine_offset << endl;
	cout << "Object offset: " << object_offset << endl;
	cout << "File size: " << file_size << endl << endl;
	
	cout << "Reading mine..." << endl;
	r.pos = mine_offset;
	
	cout << "Mine version: " << r.getc() << endl;
	
	unsigned short vertex_count = r.readShort();
	unsigned short cube_count = r.readShort();
	
	cout << "Vertex count: " << vertex_count << endl;
	cout << "Cube count: " << cube_count << endl;
	
	system("pause");
	
	for(int i = 0;i < vertex_count;i++) {
		Vertex v;
		
		v.x = r.readInt();
		v.y = r.readInt();
		v.z = r.readInt();
		
		//cout << "Read vertex: " << (v.x/65536.0) << ", " << (v.y/65536.0) << ", " << (v.z/65536.0) << endl;
		
		//system("pause");
		
		vertex.push_back(v);
	}
	
	cout << "Read verticies" << endl;
	
	for(int i = 0;i < cube_count;i++) {
		cout << "----------------------------------------" << "Cube " << i << endl << endl;
		
		Cube c;
		unsigned char mask = r.getc();
		int count = 0;
		
		cout << "Mask: " << (int)mask << endl;
		
		for(int i = 0;i < 6;i++) {
			if(mask & 1) {
				c.n[i] = r.readShort();
				count++;
				cout << "Neighbor: " << c.n[i] << endl;
			}
			else {
				c.n[i] = -64;
			}
			
			mask >>= 1;
		}
		
		cout << "Rpos: " << r.pos << endl;
		for(int i = 0;i < 8;i++) {
			c.v[i] = r.readShort();
			
			if(c.v[i] >= vertex_count) {
				cout << "Error: invalid vertex" << endl;
			}
			
			cout << "Vertex: " << c.v[i] << endl;
			cout << "V value: " << vertex[c.v[i]].x / 65536.0 << " " << vertex[c.v[i]].y / 65536.0 << " " << vertex[c.v[i]].z / 65536.0 << endl;
			
			
			//cout << "x: " << (vertex[c.v[i]].x/65536.0) << " y: " << (vertex[c.v[i]].y/65536.0) << " z: " << (vertex[c.v[i]].z/65536.0) << endl;
		}
		
		if(mask & 1) {
			r.pos += 4;
			cout << "Energy charge" << endl;
		}
		
		cout << "Light value: " << (r.readShort() / 8192.0) << endl;
		
		mask = r.getc();
		
		cout << "New mask: " << (int)mask << endl;
		
		for(int i = 0;i < 6;i++) {
			if(mask & 1) {
				c.w[i] = r.getc();
			}
			else {
				c.w[i] = -64;
			}
			
			mask >>= 1;
		}
		
		cout << "Count: " << count << endl;
		
		for(int i = 0;i < 6 - count;i++) {
			unsigned short t = r.readShort();
			
			if(t & (1 << 15)) {
				r.pos += 2;
			}
			
			r.pos += (32 + 16) * 4;
		}
		
		system("read");
		
		//break;
		//if(i > 0) break;
	}
	
	
	return true;
}
#endif

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
	
	system("read");
	
	vector<Vex3D> v;
	
	// Read in the vertices
	for(int i = 0; i < total_v; i++) {
		int x = r.readInt();
		int y = r.readInt();
		int z = r.readInt();
		
		//printf("Vex3D: {%f, %f, %f}\n", x / 65536.0, y / 65536.0, z / 65536.0);
		
		//system("pause");
		
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
			
			printf("Children %d: %d\n", d, children[d]);
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
		
		printf("Special: %d\n", special);
		
		r.readByte();		// Skip matcen_num
		r.readShort();		// Skip value
		
		r.readShort();	// Skip light value
		
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
	}
	
	if(!found_level) {
		cout << "Error: level not found" << endl;
		return 0;
	}

	level.save(name + ".xdl");
	cout << "File saved to: " << name << ".xdl" << endl;	
	
	return 0;
}