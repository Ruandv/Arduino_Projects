String Split(String str,char* deliminators,int itemNo)
{ 
    Serial.println(str);
    int v =0;
    String myArr[6];
    char * pch = strtok (str.c_str(),deliminators);
    int a=0;
    while (pch != NULL)
    { 
        Serial.println(pch);
        if(a==6) {break;}
        myArr[a] = pch;
        pch = strtok (NULL, deliminators);
        a++;
    }
    return myArr[itemNo];
} 
