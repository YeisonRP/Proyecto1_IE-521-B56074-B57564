#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>


using namespace std;

int main ()
{

    long address[10] = {4188071501,4088071500,4088071500,2081111111,1181011510,1951071615,1951011122,2189071201,198079500,1582078500};
    int index_size = 8, index, asociatividad = 4, tag_size = 20, tag, offset_size = 8, i, j, miss = 0, hit = 0;
    //index_size = address[i] << tag_size;
    //index_size = index >> (tag_size+offset_size);
    bool Hay_HIT, remplazo;

    struct entry {
        bool valid ;
        bool dirty;
        int tag ;
        uint8_t rp_value ;
    };

    struct entry cache_block[asociatividad];

    for(int a = 0; a < asociatividad; a++)
    {
        cache_block[a].valid = false;
        cache_block[a].rp_value = (asociatividad-1);
        cache_block[a].tag = 0;
    }
    
    for(i=0;i<10;i++)
    {
      //TAG 
        Hay_HIT = false;
        tag = address[i] >> (index_size+offset_size);
        cout << "Tag buscado "<< tag<<endl<<endl;
        
        for(j=0;j<asociatividad;j++)
        {
            if(cache_block[j].tag == tag && cache_block[j].valid == true)
            {
                // HAY HIT
                /*for(int m = 0; m < asociatividad; m++)
                {
                    if(cache_block[m].rp_value < (asociatividad-1)){cache_block[m].rp_value +=1;}
                }*/
                hit += 1;
                cache_block[j].rp_value = 0;  
                Hay_HIT = true;
                cout << "Hubo un HIT con el TAG "<< cache_block[j].tag<< " y su valor de remplazo es "<< (int)cache_block[j].rp_value<<endl;       
                j = asociatividad;
            }
        }
                // HAY MISS
        if(!Hay_HIT)
        {
                cout << "Hubo un miss con el TAG "<< tag<<" "<<endl;
                miss += 1;
                remplazo = false;

                while(!remplazo)
                {
                    for(int m = 0; m < asociatividad; m++)
                    {
                        if(cache_block[m].rp_value == (asociatividad-1))
                        {
                            cout << " Se reemplazará el TAG " << cache_block[m].tag << " cuyo valor de reemplazo era de "<< (int)cache_block[m].rp_value<<endl;
                            cache_block[m].valid = true;
                            cache_block[m].tag = tag;

                            cache_block[m].rp_value = (asociatividad <= 2) ? 0:2;
                            m = asociatividad;
                            remplazo = true;
                        }    
                    }    
                        // Si ninguno tiene valor de reemplazo = (asociatividad-1) se procede a aumentar los valores.
                    if(!remplazo)
                    {    
                        for(int g = 0; g < asociatividad; g++)
                        {
                            if(cache_block[g].rp_value < (asociatividad - 1)){  cache_block[g].rp_value += 1;   }  //suma 1 a la politica de remplazo
                        }
                    }   
                }
                
                
                
        
        
        
        }
        
        cout << "Estado actual del set:"<<endl;
        for(int p = 0; p<asociatividad; p++)
        {
            cout << "WAY: "<< p<<" TAG: "<<cache_block[p].tag << " Valor de reemplazo: " << (int)cache_block[p].rp_value << endl;
        }

    }
        cout << "Hubo "<<miss<<" misses y "<< hit<<" hits"<<endl; 
    /*int valor, i;
    int bin[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    cout << "Ingrese el valor."<< endl;
    cin >> valor;

    cout << "En decimal: "<< valor << endl;
    
    for(i = 0; valor > 0; i++)
    {
        bin[31-i] = valor % 2;
        valor = valor/2;
    }
    
    for(i = 0; i < 32; i++)
    {
        cout << bin[i];
    }

    cout << endl;*/
    

    return 0;

}