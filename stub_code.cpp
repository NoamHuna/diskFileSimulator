#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

#define DISK_SIZE 512

// Function to convert decimal to binary char
char decToBinary(int n) {
    return static_cast<char>(n);
}

// #define SYS_CALL
// ============================================================================
class fsInode {
    int fileSize;
    int block_in_use;
    int management_block_in_use;

    int directBlock1;
    int directBlock2;
    int directBlock3;

    int singleInDirect;
    int doubleInDirect;
    int block_size;

    int lru_block;
    int available_space_in_lru_block;

    public:
    fsInode(int _block_size) {
        fileSize = 0;
        block_in_use = 0;
        management_block_in_use = 0;
        block_size = _block_size;
        directBlock1 = -1;
        directBlock2 = -1;
        directBlock3 = -1;
        singleInDirect = -1;
        doubleInDirect = -1;

        lru_block = -1;
        available_space_in_lru_block = 0;

    }

    int get_lru_block() const{
        return lru_block;
    }
    void set_lru_block(int lru){
            lru_block = lru;
    }
    int get_available_space() const{
        return available_space_in_lru_block;
    }
    void set_available_space(int space){
            available_space_in_lru_block = space;

    }
    void save_block_details(int block, int sn){
        switch (sn) {
            case 1:
                directBlock1 = block;
                break;
            case 2:
                directBlock2 = block;
                break;
            case 3:
                directBlock3 = block;
                break;
            case 4:
                singleInDirect = block;
                break;
            case 5:
                doubleInDirect = block;
                break;
            default:
                ;
        }
    }
    int get_block_in_use() const{
        return block_in_use;
    }
    int get_mng_block_in_use(){
        return management_block_in_use;
    }
    int get_file_size(){
        return fileSize;
    }
    void set_file_size(int file_size){
        this->fileSize = file_size;
    }
    int get_block_index(int sn) const{
        switch (sn) {
            case 1:
                return directBlock1;
            case 2:
                return directBlock2;
            case 3:
                return directBlock3;
            case 4:
                return singleInDirect;
            case 5:
                return doubleInDirect;
            default:
                return -1;
        }
    }
    void increase_block_in_use(){
        block_in_use ++;
    }
    void increase_mng_block_in_use(){
        management_block_in_use ++;
    }
    void increase_file_size(){
        fileSize ++;
    }


    ~fsInode() {
        //delete directBlocks;
    }

};

// ============================================================================
class FileDescriptor {
    pair<string, fsInode*> file;
    bool inUse;

    public:
    FileDescriptor(string FileName, fsInode* fsi) {
        file.first = FileName;
        file.second = fsi;
        inUse = true;
    }



    string getFileName() {
        return file.first;
    }
    void setFileName(string name){
        this -> file.first = name;
    }

    fsInode* getInode() {

        return file.second;

    }

    int GetFileSize() {
        return this->file.second->get_file_size();
    }
    bool isInUse() {
        return (inUse);
    }
    void setInUse(bool _inUse) {
        inUse = _inUse ;
    }
};

#define DISK_SIM_FILE "DISK_SIM_FILE.txt"
// ============================================================================
class fsDisk {
    FILE *sim_disk_fd;

    bool is_formated;

	// BitVector[0] == 1, means that the first block is occupied.
    int BitVectorSize;
    int *BitVector;
    //#

    // Unix directories are lists of association structures,
    // each of which contains one filename and one inode number.
    map<string, fsInode*>  MainDir ;

    // OpenFileDescriptors -- when you open a file,
	// the operating system creates an entry to represent that file
    // This entry number is the file descriptor.
    vector< FileDescriptor > OpenFileDescriptors;

    int block_size;
    int files_count = 0;

    public:

    // ------------------------------------------------------------------------
    fsDisk() {
        sim_disk_fd = fopen( DISK_SIM_FILE , "w+");
        assert(sim_disk_fd);
        for (int i=0; i < DISK_SIZE ; i++) {
            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fwrite( "\0" ,  1 , 1, sim_disk_fd );
            assert(ret_val == 1);
        }
        fflush(sim_disk_fd);
    }



    // ------------------------------------------------------------------------
    void listAll() {
        int i = 0;
        for ( auto it = begin (OpenFileDescriptors); it != end (OpenFileDescriptors); ++it) {
            cout << "index: " << i << ": FileName: " << it->getFileName() <<  " , isInUse: "
                << it->isInUse() << " file Size: " << it->GetFileSize() << endl;
            i++;
        }
        char bufy;
        cout << "Disk content: '" ;
        for (i=0; i < DISK_SIZE ; i++) {
            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fread(  &bufy , 1 , 1, sim_disk_fd );
             cout << bufy;
        }
        cout << "'" << endl;


    }

    // ------------------------------------------------------------------------
    void fsFormat( int blockSize = 4) {
        if (is_formated) { // check if the disk has been formatted before
            // fill the disk with nulls
            for (int i = 0; i < DISK_SIZE ; i++) {
                size_t ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
                ret_val = fwrite( "\0" ,  1 , 1, sim_disk_fd );
            }
            // iterate over the Main dir, which suppose to hold all the Inodes inside it and free them as they
            // allocated by using "new"
            for(auto it = MainDir.begin(); it != MainDir.end(); it++){
                if (it -> second)
                    delete(it -> second);
            }
            free(BitVector);
            MainDir.clear();
            OpenFileDescriptors.clear();
            files_count = 0;
        }
        block_size = blockSize;
        BitVectorSize = DISK_SIZE / block_size;
        BitVector = (int *) malloc(BitVectorSize * sizeof(int));
        is_formated = true;

    }

    // ------------------------------------------------------------------------
    int CreateFile(string fileName) {
        if (!is_formated) {
            cout << "ERR" << endl;
            return -1;
        }
        auto it = MainDir.find(fileName);
        if(it != MainDir.end()) return -1; // means there is a file using this name already
        auto* temp = new fsInode(block_size); // create new fsInode
        // insert the pair to the OpenFileDescriptor and to the Main dir also, when create a file also open it
        MainDir.insert(make_pair(fileName, temp));
        // after the insertion to the MainDir open the file using the class function
        return this -> OpenFile(fileName);

    }

    // ------------------------------------------------------------------------
    int OpenFile(string FileName ) {
        if (!is_formated) {
            cout << "ERR" << endl;
            return -1;
        }
        // find if there is a file with such name, if isn't return -1
        // we will use the main dir to get the Inode of the file
        auto it = MainDir.find(FileName);
        if(it == MainDir.end()) {
            cout << "ERR" << endl;
            return -1; // the file name isn't found in the main directory
        }

        bool found = false;
        int i = 0;int index = -1;
        //iterate over the OpenFileDescriptor and look for place that already allocated
        for (; i < OpenFileDescriptors.size(); ++i) {
            if (OpenFileDescriptors[i].getFileName() == FileName && OpenFileDescriptors[i].isInUse()) {
                cout << "ERR" << endl;
                return -1;
            }
            if(!OpenFileDescriptors[i].isInUse() && !found){
                index = i;
                found = true;
            }
        }
        // if we found such, clear former and create a new one
        if (index >= 0){
            OpenFileDescriptors[index] = FileDescriptor(FileName , it -> second);
            return index;
        }
        else{
            // just add new fd at the end of the vector
            OpenFileDescriptors.emplace_back(FileName , it -> second);
            return files_count++;
        }

    }


    // ------------------------------------------------------------------------
    // todo: needs to be change, change the FileDescriptor field (in use) to false
    string CloseFile(int fd) {
        if (!is_formated) {
            cout << "ERR" << endl;
            return "-1";
        }
        // if fd is not in the range, or the file is already closed
        if(fd < 0 || fd > OpenFileDescriptors.size() || !OpenFileDescriptors[fd].isInUse()) {
            cout << "ERR" << endl;
            return "-1";
        }
        OpenFileDescriptors[fd].setInUse(false);
        return OpenFileDescriptors[fd].getFileName();
    }
    // ------------------------------------------------------------------------
    int WriteToFile(int fd, char *buf, int len ) {
        if (!is_formated) {
            cout << "ERR" << endl;
            return -1;
        }
        // validation: check if the file is existed and opened, if not, return -1
        if (fd < 0 || fd > OpenFileDescriptors.size() || !OpenFileDescriptors[fd].isInUse()){
          cout << "ERR" << endl;
          return  -1;
        }
        // cannot even start to write
        if (freeBlocksCounter() == 0){
            cout << "ERR" << endl;
            return -1;
        }
        // find how many blocks needed to this operation, find free blocks using the bit vector,
        // declare them as occupied, write the content and update the block information of the file
        int index = 0;
        int available_space;
        fsInode* Inode = OpenFileDescriptors[fd].getInode();
        // write to available space in the last block that already allocated to this file
        int space = Inode->get_available_space();
        if (space > 0) {
            int block_write_to = Inode->get_lru_block();
            int start_offset = block_write_to * block_size + (block_size - space);
            // locate the cursor in the location which will continue the last writing
            for (int i = 0; i < space; i++) {
                fseek(sim_disk_fd, start_offset + i, SEEK_SET);
                size_t ret = fwrite(&buf[index], sizeof(char), 1, sim_disk_fd);
                if(ret != 1){
                    return index;
                }
                Inode -> increase_file_size();
                index++;
                if (index == len) {
                    // set the remaining space to be the difference, don't change the lru block
                    available_space = space - len;
                    Inode ->set_available_space(available_space);
                    return len;
                }
            }
        }

        // allocate blocks, enough to cover the content that left to write
        int reminder = (len - index) % block_size; // potentially unused space
        int blocks_to_alloc  = (len - index) / block_size;;
        if (reminder == 0){ // we will use all the memory we will allocate
            available_space = 0;
        }
        else{
            blocks_to_alloc++;
            available_space= block_size - reminder;
        }
        int allocated;
        for (int i = 0; i < blocks_to_alloc; ++i) {
            allocated = getBlock(); // allocate block in order to write to it
            if (allocated == -1) {
                available_space = 0;
                break;
            }
            // first of all, save the index of the allocated block in the Inode
            if (save_block(allocated, Inode) == -1) {
                BitVector[allocated] = 0;
                available_space = 0;
                break;
            }
            int start_offset = allocated * block_size;
            for (int j = 0; j < block_size; j++) {
                fseek(sim_disk_fd, start_offset + j, SEEK_SET);
                size_t ret = fwrite(&buf[index], sizeof(char), 1, sim_disk_fd);
                Inode -> increase_file_size();
                if(ret != 1){
                    return index;
                }
                index++;
                if(index == len){
                    // finish writing all the requested data
                    break;
                }
            }
        }
        // save the data in order to help us, if possible, use the whole block next time
        Inode ->set_available_space(available_space);
        Inode ->set_lru_block(allocated);
        return index;

    }
    int save_block(int allocated, fsInode* Inode) {
        // the allocated block is the on where we will write in the data.
        // therefore, we will check what kind of pointer we got for the block
        int block_in_use = Inode->get_block_in_use();

        //for the directed blocks, save them as is in the Inode
        if (block_in_use == 0){
            Inode -> save_block_details(allocated, 1);
        }
        else if(block_in_use == 1){
            Inode -> save_block_details(allocated, 2);
        }
        else if(block_in_use == 2 ){
            Inode -> save_block_details(allocated, 3);
        }
        else if(block_in_use == 3){
            // alloc another block for info management, single indirect
            int single_indirect = getBlock();
            if (single_indirect == -1) return -1;
            Inode -> increase_mng_block_in_use();
            Inode -> save_block_details(single_indirect, 4);
            fseek(sim_disk_fd, block_size * single_indirect, SEEK_SET);
            fputc(decToBinary(allocated), sim_disk_fd);
        }
        //using the one inDirect block
        else if(block_in_use > 3 && block_in_use < 3 + block_size){
            int single_indirection = Inode -> get_block_index(4); // use the single indirect block from the Inode
            fseek(sim_disk_fd, block_size * single_indirection + (block_in_use - 3), SEEK_SET);
            fputc(decToBinary(allocated), sim_disk_fd);
        }
        else if(block_in_use == 3 + block_size) {
            // alloc another block for info management or even two
            int first_twoInDirect = getBlock(); // first level of the double in-directed blocks
            if (first_twoInDirect == -1) return -1;
            Inode -> increase_mng_block_in_use();
            int second_twoInDirect = getBlock();
            if (second_twoInDirect == -1){
                BitVector[first_twoInDirect] = 0;
                return -1;
            }
            Inode -> increase_mng_block_in_use();
            Inode->save_block_details(first_twoInDirect, 5);
            // in the double indirect block save the index of the more close level
            fseek(sim_disk_fd, block_size * first_twoInDirect, SEEK_SET);
            fputc(decToBinary(second_twoInDirect), sim_disk_fd);
            //then save in the closer level the index of the block with the data
            fseek(sim_disk_fd, block_size * second_twoInDirect, SEEK_SET);
            fputc(decToBinary(allocated), sim_disk_fd);
        }
        else if (block_in_use > block_size + 3 && block_in_use < block_size * block_size + block_size + 3){
            if ((block_in_use - ( 3 + block_size)) % block_size == 0){
                int two_level_index = (block_in_use - (3 + block_size)) / block_size; // index in the further block
                int first_level_index = 0; // if we needed to alloc new block, the index of the closer block will be 0
                int first_level_block = getBlock(); //  allocate one more block
                if (first_level_block == -1) return -1;
                Inode -> increase_mng_block_in_use();
                fseek(sim_disk_fd, block_size * Inode ->get_block_index(5) + two_level_index,
                      SEEK_SET);
                fputc(decToBinary(first_level_block), sim_disk_fd);
                fseek(sim_disk_fd, block_size * first_level_block, SEEK_SET);
                fputc(decToBinary(allocated), sim_disk_fd); //eventually save the block containing the data
            }
            else{
                // use existing closer index-block
                int two_level_index = (block_in_use - (3 + block_size)) / block_size;
                int first_level_index = (block_in_use - (3 + block_size)) % block_size;
                fseek(sim_disk_fd, block_size * Inode ->get_block_index(5) + two_level_index,
                      SEEK_SET);
                int first_level_block = fgetc(sim_disk_fd);
                fseek(sim_disk_fd, block_size * first_level_block + first_level_index, SEEK_SET);
                fputc(decToBinary(allocated), sim_disk_fd);
            }
        }
        else{
            // maximum blocks
            return -1;
        }
        Inode -> increase_block_in_use();
        return 1;

    }
    // ------------------------------------------------------------------------
    int DelFile(string FileName) {
        if (!is_formated) {
            cout << "ERR" << endl;
            return -1;
        }
        // pass over the Inode, release the blocks occupied by this file
        auto it = MainDir.find(FileName);
        if(it == MainDir.end())
        {
            cout << "ERR" << endl;
            return -1; // the file name isn't found in the main directory
        }

        //iterate over the OpenFileDescriptor and look for place that already allocated
        for (auto & OpenFileDescriptor : OpenFileDescriptors) {
            if(OpenFileDescriptor.getFileName() == FileName) {
                if (OpenFileDescriptor.isInUse()){
                    cout << "ERR" << endl;
                    return -1;
                }
                else {
                    RenameFile(FileName, "");
                    MainDir.erase("");
                    break;
                }
            }
        }
        // declare all the blocks that the file used as free to alloc blocks
        fsInode * Inode = it->second;
        int blocks_to_free = Inode -> get_block_in_use();
        for (int i = 1; i < 4; ++i) {
            BitVector[Inode ->get_block_index(i )] = 0;
            blocks_to_free --;
            if (blocks_to_free == 0){
                delete Inode;
                return 1;
            }
        }
        int single_indirect = Inode ->get_block_index(4);
        for (int i = 0; i < block_size; ++i) {
            fseek(sim_disk_fd, block_size * single_indirect + i, SEEK_SET);
            BitVector[fgetc(sim_disk_fd)] = 0;
            blocks_to_free--;
            if (blocks_to_free == 0) { // still single_indirect management left
                BitVector[single_indirect] = 0;
                delete Inode;
                return 1; // if we finished declaring the blocks as free to use
            }
        }
        BitVector[single_indirect] = 0;
        int second_double_indirect = Inode ->get_block_index(5);
        for (int i = 0; i < block_size; ++i) {
            fseek(sim_disk_fd, block_size * second_double_indirect + i, SEEK_SET);
            int first_double_indirect = fgetc(sim_disk_fd); // get the closer block index
            for (int j = 0; j < block_size; ++j) {
                fseek(sim_disk_fd, block_size * first_double_indirect + j, SEEK_SET);
                BitVector[fgetc(sim_disk_fd)] = 0;
                blocks_to_free --;
                if (blocks_to_free == 2){
                    BitVector[first_double_indirect] = 0;
                    BitVector[second_double_indirect] = 0;
                    delete Inode;
                    return 1;
                }
            }
            BitVector[first_double_indirect] = 0;
        }
        BitVector[second_double_indirect] = 0;
        delete Inode;
        return 1;
    }
    // ------------------------------------------------------------------------
    int ReadFromFile(int fd, char *buf, int len) {
        if (!is_formated) {
            cout << "ERR" << endl;
            return -1;
        }
        // find how many blocks demanded, get their indices from the match fs_inode
        buf[0] = '\0';
        int index = 0;
        if (fd < 0 || fd > OpenFileDescriptors.size() || !OpenFileDescriptors[fd].isInUse()) {
            cout << "ERR" << endl;
            return -1;
        }
        fsInode* Inode = OpenFileDescriptors[fd].getInode();
        len = len <= Inode -> get_file_size() ? len : Inode -> get_file_size();
        if (len <= 0) return 0;
        int blocks_to_read = len % block_size == 0 ? len / block_size : len / block_size + 1;
        int level = 1;
        int limit_1 = blocks_to_read < 3 ? blocks_to_read : 3;
        int i = 0;
        for (; i < limit_1; ++i) {
            int ith_direct = Inode->get_block_index(level);
            for (int j = 0; j < block_size; ++j) {
                fseek(sim_disk_fd, block_size * ith_direct + j, SEEK_SET);
                buf[index++] = static_cast<char>(fgetc(sim_disk_fd));
                if (index == len) {
                    buf[index] = '\0';
                    return index;
                }
            }
            level ++;
        }
        blocks_to_read -= 3;
        // continue to singleInDirect
        int limit_2 = blocks_to_read <  block_size ? blocks_to_read : block_size;
        int singleInDirect = Inode ->get_block_index(level);
        for (int j = 0; j < limit_2; ++j) {
            fseek(sim_disk_fd, block_size * singleInDirect + j, SEEK_SET);
            int block = fgetc(sim_disk_fd);
            for (int k = 0; k < block_size; ++k) {
                fseek(sim_disk_fd, block_size * block + k,SEEK_SET);
                buf[index++] = static_cast<char>(fgetc(sim_disk_fd));
                if (index == len){
                    buf[index] = '\0';
                    return index;
                }
            }
            i++;
        }
        level ++;
        blocks_to_read -= block_size;
        //continue to doubleInDirect
        //int limit_3 = blocks_to_read < block_size * block_size ? blocks_to_read : block_size * block_size;
        int second_level_twoInDirect = Inode ->get_block_index(level);
        for (int j = 0; j < block_size; ++j) { // iterate over the further index block
            fseek(sim_disk_fd, block_size * second_level_twoInDirect + j, SEEK_SET);
            int first_block = fgetc(sim_disk_fd); // get the index of the closer block
            for (int k = 0; k < block_size; ++k) { // iterate over the closer index block
                fseek(sim_disk_fd, block_size * first_block + k, SEEK_SET);
                int block = fgetc(sim_disk_fd);
                for (int l = 0; l < block_size; ++l) { // read from data block
                    fseek(sim_disk_fd, block_size * block + l, SEEK_SET);
                    buf[index++] = static_cast<char>(fgetc(sim_disk_fd));
                    if (index == len) {
                        buf[index] = '\0';
                        return index;
                    }
                }
                blocks_to_read --;
            }
        }
        buf[index] = '\0';
        return index;
    }

    // ------------------------------------------------------------------------
    int GetFileSize(int fd) {
        if (!is_formated) {
            cout << "ERR" << endl;
            return -1;
        }
        if (fd < 0 || fd > OpenFileDescriptors.size() || !OpenFileDescriptors[fd].isInUse()) {
            cout << "ERR" << endl;
            return -1;
        }
        return OpenFileDescriptors[fd].getInode() -> get_file_size();
    }

    // ------------------------------------------------------------------------
    int CopyFile(string srcFileName, string destFileName) {
        if (!is_formated) {
            cout << "ERR" << endl;
            return -1;
        }
        auto it_1 = MainDir.find(srcFileName);
        if (it_1 == MainDir.end()) {
            cout << "ERR" << endl;
            return -1;
        }
        for (FileDescriptor file : OpenFileDescriptors) {
            if (file.getFileName() == srcFileName){
                if (file.isInUse()) {
                    cout << "ERR" << endl;
                    return -1;
                } else
                    break;
            }
        }
        fsInode* source = it_1 -> second;
        auto it_2 = MainDir.find(destFileName);
        int src_blocks = it_1 -> second->get_block_in_use() + it_1 -> second -> get_mng_block_in_use();
        int free_blocks = freeBlocksCounter();
        if (it_2 != MainDir.end()) {
            // check if we will have enough space to copy all the content
            int dst_blocks = it_2->second->get_block_in_use() + it_2->second->get_mng_block_in_use();
            if (free_blocks + dst_blocks < src_blocks) {
                cout << "ERR" << endl;
                return -1;
            }
            DelFile(destFileName);
        }
        else if (src_blocks > free_blocks) {
            cout << "ERR" << endl;
        }
        int fd;
        fd = CreateFile(std::move(destFileName));
        if(fd == -1) {
            cout << "ERR" << endl;
            return -1;
        }
        int len = source -> get_file_size();
        char buf[len + 1];// one more char to '\0'
        int index = OpenFile(srcFileName);
        if (index == -1){
            CloseFile(fd);
            cout << "ERR" << endl;
            return -1;
        }
        if (ReadFromFile(index, buf, len) == -1) {
            cout << "ERR" << endl;
            return -1;
        } ;
        if (WriteToFile(fd, buf, len) == -1){
            cout << "ERR" << endl;
            return -1;
        }
        if (CloseFile(index) == "-1"){
            cout << "ERR" << endl;
            return -1;
        }
        if (CloseFile(fd) == "-1"){
            cout << "ERR" << endl;
            return -1;
        }
        return 1;
    }

    // ------------------------------------------------------------------------

    int RenameFile(string oldFileName, string newFileName) {
        if (!is_formated) {
            cout << "ERR" << endl;
            return -1;
        }
        auto it = MainDir.find(oldFileName);
        if (it == MainDir.end()){
            cout << "ERR" << endl;
            return -1; // The file name isn't found in the main directory
        }
        auto it_2 = MainDir.find(newFileName);
        if (it_2 != MainDir.end()){
            cout << "ERR" << endl;
            return -1; // The file name isn't found in the main directory
        }
        //want to make sure the file isn't open, and check if the file is part of the OpenFileDesc vector
        int i = 0; bool included = false;
        for (; i < OpenFileDescriptors.size(); ++i) {
            // if the file is open name cannot be changed
            if (OpenFileDescriptors[i].getFileName() == oldFileName) {
                if (OpenFileDescriptors[i].isInUse()) {
                    cout << "ERR" << endl;
                    return -1;
                }
                else{
                    included = true;
                    break;
                }

            }
        }

        // Remove the old entry from the map, save a pointer to the fs_Inode before deleting
        // it there it takes place in the OpenFileDescriptors and we reach till here we fan ensure its closed
        fsInode* temp = it -> second;
        MainDir.erase(it);
        if (included) OpenFileDescriptors[i].setFileName(newFileName);
        // Insert a new entry with the new key and the same inode pointer
        MainDir.insert(make_pair(newFileName, temp));

        return 1;
    }
    ~fsDisk(){
        // iterate over the MainDir which suppose to hold all the inode pointers
        for(auto & it : MainDir){
            if (!it.second)
                delete(it.second);
        }
        free(BitVector);
        MainDir.clear();
        OpenFileDescriptors.clear();
        fclose(sim_disk_fd);
    }

    int getBlock(){
        // iterate over the bit vector look for unused block, declare as use and return the index
        for(int  i = 0 ; i < BitVectorSize ; i++){
            if(BitVector[i] == 0){
                BitVector[i] = 1;
                return i;
            }
        }
        return -1;
    }
    int freeBlocksCounter(){
        // count how much free blocks we have
        int counter = 0;
        for (int i = 0; i < BitVectorSize ; ++i) {
            if (BitVector[i] == 0) counter++;
        }
        return counter;
    }

};

int main() {
    int blockSize;
    int direct_entries;
    string fileName;
    string fileName2;
    char str_to_write[DISK_SIZE];
    char str_to_read[DISK_SIZE];
    int size_to_read;
    int _fd;

    fsDisk *fs = new fsDisk();
    int cmd_;
    while(1) {
        cin >> cmd_;
        switch (cmd_)
        {
            case 0:   // exit
                delete fs;
                exit(0);
                break;

            case 1:  // list-file
                fs->listAll();
                break;

            case 2:    // format
                cin >> blockSize;
                fs->fsFormat(blockSize);
                break;

            case 3:    // creat-file
                cin >> fileName;
                _fd = fs->CreateFile(fileName);
                cout << "CreateFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 4:  // open-file
                cin >> fileName;
                _fd = fs->OpenFile(fileName);
                cout << "OpenFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 5:  // close-file
                cin >> _fd;
                fileName = fs->CloseFile(_fd);
                cout << "CloseFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 6:   // write-file
                cin >> _fd;
                cin >> str_to_write;
                fs->WriteToFile( _fd , str_to_write , strlen(str_to_write) );
                break;

            case 7:    // read-file
                cin >> _fd;
                cin >> size_to_read ;
                fs->ReadFromFile( _fd , str_to_read , size_to_read );
                cout << "ReadFromFile: " << str_to_read << endl;
                break;

            case 8:   // delete file
                cin >> fileName;
                _fd = fs->DelFile(fileName);
                cout << "DeletedFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 9:   // copy file
                cin >> fileName;
                cin >> fileName2;
                fs->CopyFile(fileName, fileName2);
                break;

            case 10:  // rename file
                cin >> fileName;
                cin >> fileName2;
                fs->RenameFile(fileName, fileName2);
                break;

            default:
                break;
        }
    }

}

