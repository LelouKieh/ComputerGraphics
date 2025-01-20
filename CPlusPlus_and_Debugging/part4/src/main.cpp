// Include our custom library
#include "PPM.hpp"

void unitTest1(){
    // Darken Test
    PPM myPPM("./../../common/textures/big_buck_bunny_blender3d.ppm");
    myPPM.darken();
    myPPM.savePPM("./darken.ppm");
}

void unitTest2(){
    // Lighten Test
    PPM myPPM2("./../../common/textures/big_buck_bunny_blender3d.ppm");
    myPPM2.lighten();
    myPPM2.savePPM("./lighten.ppm");    
}
 
void unitTest3(){
    // More difficult parsing test
    PPM myPPM3("./../../common/textures/big_buck_bunny_blender3d_with_weird_formatting.ppm");
    myPPM3.lighten();
    myPPM3.savePPM("./parse.ppm"); 
}

void unitTest4(){
    // My Darken Test
    PPM myPPM("./test/example.ppm");
    myPPM.darken();
    myPPM.savePPM("./mydarken.ppm");
}

void unitTest5(){
    // My Lighten Test
    PPM myPPM2("./test/example.ppm");
    myPPM2.lighten();
    myPPM2.savePPM("./mylighten.ppm");    
}
 
void unitTest6(){
    // Some values larger than 255 and some values less than 0 are in this test file
    PPM myPPM3("./test/example_weird_format.ppm");
    myPPM3.lighten();
    myPPM3.savePPM("./myparse.ppm"); 
}

void unitTest7(){
    // PPMs that have comments in them
    PPM myPPM2("./test/ppm_with_comments.ppm");
    myPPM2.lighten();
    myPPM2.savePPM("./mytestppm_with_comments.ppm");    
}

void unitTest8(){
    // Parse multiple RGB values on a line without breaking the parser
    PPM myPPM2("./test/ppm_with_multi_rgb_values.ppm");
    myPPM2.lighten();
    myPPM2.savePPM("./mytestppm_with_multi_rgb.ppm");    
}

void unitTest9() {
    // Test for a PPM with a magic number that is not P3 or P6
    PPM myPPM2("./test/example_wrong_magic_num.ppm");
    myPPM2.lighten();
    myPPM2.savePPM("./mytest_wrong_p5.ppm"); 
}

void unitTest10() {
    // Test for a PPM with P6 magic number
    PPM myPPM2("./test/example_p6.ppm");
    myPPM2.lighten();
    myPPM2.savePPM("./myest_p6.ppm"); 
}


int main(int argc, char* argv[]){

    // unitTest1();
    // unitTest2();
    // unitTest3();
    unitTest4();
    unitTest5();
    unitTest6();
    unitTest7();
    unitTest8();
    unitTest9();
    unitTest10();
    
    return 0;
}
