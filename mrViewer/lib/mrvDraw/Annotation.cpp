

#include "Annotation.h"

namespace tl
{
    namespace draw
    {
        struct Annotation::Private
        {
            int64_t frame = std::numeric_limits<int64_t>::max();
            std::vector< std::shared_ptr< Shape > > shapes;
            std::vector< std::shared_ptr< Shape > > undo_shapes;
        };

        Annotation::Annotation( const int64_t frame ) :
            _p( new Private )
        {
            TLRENDER_P();

            p.frame = frame;
        }
        
        Annotation::~Annotation()
        {
        }
        
        int64_t Annotation::frame() const
        {
            return _p->frame;
        }
        
        void Annotation::push_back(const std::shared_ptr< Shape >& shape)
        {
            _p->shapes.push_back( shape );
        }
        

        const std::vector< std::shared_ptr< Shape > >&
        Annotation::shapes() const
        {
            return _p->shapes;
        }
        
        std::shared_ptr< Shape > Annotation::lastShape() const
        {
            if ( _p->shapes.empty() )
                throw std::runtime_error( "Annotation shapes are empty" );
            return _p->shapes.back();
        }
        
        void Annotation::undo()
        {
            TLRENDER_P();
            
            if ( p.shapes.empty() ) return;
            
            const auto& shape = p.shapes.back();
            p.undo_shapes.push_back( shape );
            p.shapes.pop_back();
        }
        
        void Annotation::redo()
        {
            TLRENDER_P();
            
            if ( p.undo_shapes.empty() ) return;
            
            const auto& shape = p.undo_shapes.back();
            p.shapes.push_back( shape );
            p.undo_shapes.pop_back();
        }
    }
}
