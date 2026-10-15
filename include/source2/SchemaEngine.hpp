#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <array>

namespace Inkwell::Source2
{
    struct SchemaClassFieldData_t
    {
        char* field_name;
        void* field_type;
        int field_offset;
    private:
        char pad[0xC];
    };

    class CSchemaClassBinding
    {
    public:
        const char* GetBindingName() const 
        { 
            return *reinterpret_cast<const char* const*>( reinterpret_cast<uintptr_t>( this ) + 0x8 ); 
        }
        unsigned short GetDataArraySize() const 
        { 
            return *reinterpret_cast<const unsigned short*>( reinterpret_cast<uintptr_t>( this ) + 0x1C ); 
        }
        SchemaClassFieldData_t* GetDataArray() const 
        { 
            return *reinterpret_cast<SchemaClassFieldData_t**>( reinterpret_cast<uintptr_t>( this ) + 0x28 ); 
        }
    };

    class CSchemaList
    {
    public:
        class SchemaBlock
        {
        public:
            SchemaBlock* Next() const { return m_next; }
            CSchemaClassBinding* GetBinding() const { return m_binding; }
        private:
            void* unk0;
            SchemaBlock* m_next;
            CSchemaClassBinding* m_binding;
        };

        class BlockContainer
        {
        public:
            SchemaBlock* GetFirstBlock() const { return m_first; }
        private:
            void* unk[2];
            SchemaBlock* m_first;
        };

        const std::array<BlockContainer, 256>& GetContainers() const
        {
            return *reinterpret_cast<const std::array<BlockContainer, 256>*>( this );
        }
    };

    class CSchemaSystemTypeScope
    {
    public:
        const char* GetScopeName() const
        {
            return reinterpret_cast<const char*>( reinterpret_cast<uintptr_t>( this ) + 0x8 );
        }

        CSchemaList* GetClassContainer()
        {
            return reinterpret_cast<CSchemaList*>( reinterpret_cast<uintptr_t>( this ) + 0x5C0 );
        }
    };

    class ISchemaSystem
    {
    public:
        virtual void* Placeholder0() = 0;
        virtual void* Placeholder1() = 0;
        virtual void* Placeholder2() = 0;
        virtual void* Placeholder3() = 0;
        virtual void* Placeholder4() = 0;
        virtual void* Placeholder5() = 0;
        virtual void* Placeholder6() = 0;
        virtual void* Placeholder7() = 0;
        virtual void* Placeholder8() = 0;
        virtual void* Placeholder9() = 0;
        virtual void* Placeholder10() = 0;
        virtual CSchemaSystemTypeScope* GlobalTypeScope() = 0;
        virtual void* Placeholder12() = 0;
        virtual CSchemaSystemTypeScope* FindTypeScopeForModule( const char* module_name , void* unk = nullptr ) = 0;
    };

    class SchemaEngine
    {
    public:
        static auto Get() -> SchemaEngine&;

        auto Initialize() -> bool;
        auto GetOffset( std::string_view class_name , std::string_view prop_name ) -> uint32_t;
        auto RegisterStaticOffset( std::string_view class_name , std::string_view prop_name , uint32_t offset ) -> void;
        auto DumpSchemaToFile( const std::string& output_path ) -> bool;

    private:
        SchemaEngine() = default;

        auto CrawlTypeScope( CSchemaSystemTypeScope* scope ) -> void;

        std::unordered_map<std::string , std::unordered_map<std::string , uint32_t>> m_schema_cache;
        bool m_is_initialized = false;
        size_t m_crawled_fields_count = 0;
        size_t m_crawled_classes_count = 0;
    };
}

#define INK_SCHEMA(class_name, prop_name, type) \
    inline auto prop_name() -> type& { \
        static uint32_t s_offset = 0; \
        if ( s_offset == 0 ) { \
            s_offset = Inkwell::Source2::SchemaEngine::Get().GetOffset(#class_name, #prop_name); \
        } \
        static type s_dummy{}; \
        if ( s_offset == 0 ) return s_dummy; \
        return *reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(this) + s_offset); \
    }

#define INK_PSCHEMA(class_name, prop_name, type) \
    inline auto prop_name() -> type* { \
        static uint32_t s_offset = 0; \
        if ( s_offset == 0 ) { \
            s_offset = Inkwell::Source2::SchemaEngine::Get().GetOffset(#class_name, #prop_name); \
        } \
        if ( s_offset == 0 ) return nullptr; \
        return *reinterpret_cast<type**>(reinterpret_cast<uintptr_t>(this) + s_offset); \
    }
