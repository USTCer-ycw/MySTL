//
//  TinySTL_alloc.h
//  MyStl
//
//  Created by 姚传望 on 2020/4/24.
//  Copyright © 2020 姚传望. All rights reserved.
//

#ifndef TinySTL_alloc_h
#define TinySTL_alloc_h
#define __THROW_BAD_ALLOC throw bad_alloc//std::cerr<<"out of memory"<<std::endl;exit(1);
//#ifndef _THROW_BAD_ALLOC
//#include <new>
//#define _THROW_BAD_ALLOC throw bad_alloc
//#endif

//配置第一级配置器
//小于128bytes时使用
namespace TinySTL {

template <int inst>
class _malloc_alloc_template {
private:
    //oom out_of_memory
    static void* oom_malloc(size_t);
    static void* oom_remalloc(void*,size_t);
    static void (*_malloc_alloc_oom_handler) ();//指向void()的函数指针
public:
    //第一级配置器直接使用malloc
    static void* allocate(size_t n){
        void* result = malloc(n);
        if(!result) result = oom_malloc(n);
        return result;
    }
    
    static void deallocate(void* p,size_t){//占位符形参size_t
        free(p);//第一级直接使用malloc和free
    }
    
    static void* reallocate(void* p,size_t old_size,size_t new_size){
        void* result = realloc(p,new_size);
        if(!result) result = oom_remalloc(p, new_size);
        return result;
    }
    //分配失败调用
    //这里定义了一个函数set_malloc_handler，它接受一个void (*)()类型的参数f，返回类型为void (*)()。
//    static void (* set_malloc_handler(void (*f) ())) ()
//    {
//        void (*old)() = _malloc_alloc_oom_handler;
//        _malloc_alloc_oom_handler = f;
//        return (old);
//    }
    //与上面的函数功能一样，下面的写法更通俗易懂
    typedef void (*PF)(); //我们定义一个函数指针类型PF代表void (*)()类型
    static PF set_malloc_handler(PF f) {
        PF old = _malloc_alloc_oom_handler;
        _malloc_alloc_oom_handler = f;
        return (old);
    }
    
};
template<int inst>
void (*_malloc_alloc_template<inst>::_malloc_alloc_oom_handler) () = NULL;//初始化函数指针

template <int inst>
void * _malloc_alloc_template<inst>::oom_malloc(size_t n){
    void (* my_malloc_handler) ();
    void* result;
    while(true){
        my_malloc_handler = _malloc_alloc_oom_handler;
        if(!my_malloc_handler) _THROW_BAD_ALLOC;
        (*my_malloc_handler) ();//调用函数指针,尝试释放内存
        result = malloc(n); //再次尝试分配
        if(result) return result;
    }
}

template <int inst>
void* _malloc_alloc_template<inst>::oom_remalloc(void* p,size_t n){
    void (* my_malloc_handler) ();
    void* result;
    while(true){
        my_malloc_handler = _malloc_alloc_oom_handler;
        if(!my_malloc_handler) _THROW_BAD_ALLOC;
        (*my_malloc_handler) (); //尝试释放内存
        result = realloc(p, n); //再次分配
        if(result) return (result);
    }
}

typedef _malloc_alloc_template<0> malloc_alloc;
//typedef malloc_alloc alloc;

template<class T,class Alloc>
class simple_alloc{
public:
    static T* allocate(size_t n){
        return n==0?0:(T*)Alloc::allocate(n*sizeof(T));
    }
    static T* allocate(void){
        return (T*)Alloc::allocate(sizeof(T));
    }
    static void deallocate(T* p,size_t n){
        if(n!=0) Alloc::deallocate(p,n*sizeof(T));
    }
    static void deallocate(T* p){
        Alloc::deallocate(p,sizeof(T));
    }
};
//下面配置第二级配置器

//第一参数用于多线程
template<bool threads, int inst>
class _default_alloc_template {
private:
    static const int _ALIGN = 8;//8字节对齐
    static const int _MAX_BYTES = 128;//最大的区块大小
    static const int _NFREELISTS = _MAX_BYTES/_ALIGN;//自由链表个数,16个
private:
    static size_t ROUND_UP(size_t bytes){
        return ((bytes)+_ALIGN-1) &~ (_ALIGN-1) ;//后3位全0，与8对齐,向上取整
    }
    union obj{
        union obj* free_list_link;//4字节指针,64位8字节
        char client_data[1];//1字节
    };
private:
    //创建16个lists指针
    static obj* volatile free_list[_NFREELISTS];
    //根据区块大小，决定使用第n号list，n从1计算
    static size_t FREELIST_INDEX(size_t bytes){
        return (bytes+_ALIGN-1)/(_ALIGN-1);
    }
    //返回一个大小为n第对象，并可能加入大小为n第其他区块到list
    static void* refill(size_t n);
    //配置一大块空间，可以容纳nobj个大小为size的区块
    static char* chunk_alloc(size_t size,int& nobjs);
    //chunk allocation state
    static char* start_free ;//内存池起始位置
    static char* end_free;
    static size_t heap_size;
public:
    static void* allocate(size_t n){
        if(n>(size_t)_MAX_BYTES){
            return malloc_alloc::allocate(n);
        }
        else{
            /*obj** my_free_list，这样的话*my_free_list（空闲的内存块指针数组中的一个元素）可能被优化到寄存器中，从而使库代码无法lock住对它的读调用（如果在寄存器中则另一个线程可能会无意中修改该寄存器的值，而在内存中由于另一个线程没有访问权力所以不能修改）。
            要声明变量必须在内存中就要用volatile修饰，这里修饰的是*my_free_list，是free_list数组中的一个元素，而不是数组指针，所以volatile放在两个*中间。
             */
            obj* volatile* my_free_list;//指向指针的指针
            obj* result;
            
            my_free_list = free_list+ FREELIST_INDEX(n); //free_listz为链表首地址，后面的是偏移量
            //my_free_list指向将要填充的obj
            result = *my_free_list;
            
            if(result==NULL){//如果未找到
                void* re = refill(ROUND_UP(n));
                return re;
            }
            
            *my_free_list = result->free_list_link;
            return result;
        }
    }
    
    static void deallocate(void* p,size_t n){
        obj* q = (obj*) p;
        obj* volatile* my_free_list;
        if(n>(size_t)_MAX_BYTES){
            malloc_alloc::deallocate(p, n);
            return ;
        }
        else{
            my_free_list = free_list + FREELIST_INDEX(n);
            q->free_list_link = *my_free_list;
            *my_free_list = q;//重新指向链表头，即下一次分配的位置，并没有实际删除
        }
    }
    
    static void* reallocate(void* p,size_t old_size,size_t new_size){
        void* result;
        size_t copy_size;
        if(old_size>(size_t)_MAX_BYTES && new_size>(size_t)_MAX_BYTES) return  realloc(p,new_size);
        if(ROUND_UP(old_size) == ROUND_UP(new_size)) return p;
        result = allocate(new_size);
        copy_size = new_size >old_size?old_size:new_size;
        memcpy(result, p, copy_size);
        deallocate(p, old_size);
        return result;
    }
};

template<bool threads,int inst>
char* _default_alloc_template<threads,inst>::start_free=0;

template<bool threads,int inst>
char* _default_alloc_template<threads,inst>::end_free=0;

template<bool threads,int inst>
size_t _default_alloc_template<threads,inst>::heap_size=0;

template<bool threads,int inst>
typename _default_alloc_template<threads, inst>::obj* volatile _default_alloc_template<threads,inst>::free_list[_NFREELISTS] =
{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

template<bool threads,int inst>
void* _default_alloc_template<threads,inst>::refill(size_t n){
    int nobjs=20;
    //尝试取得20个新节点
    char* chunk = chunk_alloc(n, nobjs);
    obj* volatile* my_free_list;
    obj* result;
    obj* current_obj,*next_obj;
    if(nobjs==1) return chunk;
    
    my_free_list = free_list + FREELIST_INDEX(n);
    
    result = (obj*)chunk;
    *my_free_list = next_obj = (obj*)(chunk+n);
    for(int i=1;;i++){
        current_obj=next_obj;
        next_obj=(obj*)((char*)next_obj+n);
        if(nobjs-1==i){
            current_obj->free_list_link =0;
            break;
        }
        else{
            current_obj->free_list_link=next_obj;
        }
    }
    return result;
}

template<bool threads,int inst>
char* _default_alloc_template<threads,inst>::chunk_alloc(size_t size, int& nobjs){
    char* result;
    size_t total_bytes = size*nobjs;
    size_t bytes_left = end_free - start_free;//内存池剩余空间
    
    if(bytes_left>=total_bytes){ // 剩余空间满足需求
        result = start_free;
        start_free += total_bytes;
        return result;
    }
    else if(bytes_left>=size){//不足，但可以供应一个以上的区块ll
        nobjs=bytes_left/size;
        total_bytes=size*nobjs;
        result = start_free;
        start_free+=total_bytes;
        return result;
    }
    else {
        size_t bytes_to_get = 2*total_bytes+ ROUND_UP(heap_size>>4);
        if(bytes_left>0){
            obj* volatile* my_free_list = free_list+FREELIST_INDEX(bytes_left);
            ((obj*)start_free)->free_list_link=*my_free_list;
            *my_free_list = (obj*)start_free;
        }
        start_free = (char*)malloc(bytes_to_get);
        if(start_free==0){
            obj* volatile* my_free_list,*p;
            for(int i=size;i<=_MAX_BYTES;i+=_ALIGN){
                my_free_list = free_list + FREELIST_INDEX(i);
                p=*my_free_list;
                if(p!=0){
                    *my_free_list = p->free_list_link;
                    start_free=(char*)p;
                    end_free=start_free+i;
                    return (chunk_alloc(size, nobjs));
                }
            }
            end_free = 0;
            start_free = (char*) malloc_alloc::allocate(bytes_to_get);
        }
        heap_size+=bytes_to_get;
        end_free=start_free+bytes_to_get;
        return (chunk_alloc(size, nobjs));
    }
}
#ifdef __USE_MALLOC
typedef malloc_alloc alloc;
#else
typedef _default_alloc_template<false, 0> alloc;
#endif
}
#endif /* TinySTL_alloc_h */
