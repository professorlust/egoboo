#include "game/Graphics/RenderPasses/OpaqueEntitiesRenderPass.hpp"
#include "game/Module/Module.hpp"
#include "game/graphic_mad.h"
#include "game/graphic_prt.h"
#include "egolib/Entities/_Include.hpp"

namespace Ego {
namespace Graphics {

OpaqueEntitiesRenderPass::OpaqueEntitiesRenderPass() :
    RenderPass("opaque entities")
{}

void OpaqueEntitiesRenderPass::doRun(::Camera& camera, const TileList& tl, const EntityList& el)
{
    OpenGL::PushAttrib pa(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    {
        // scan for solid objects
        for (size_t i = 0, n = el.getSize(); i < n; ++i)
        {
            auto& renderer = Renderer::get();
            // solid objects draw into the depth buffer for hidden surface removal
            renderer.setDepthWriteEnabled(true);

            // do not draw hidden surfaces
            renderer.setDepthTestEnabled(true);
            renderer.setDepthFunction(CompareFunction::Less);

            renderer.setAlphaTestEnabled(true);
            renderer.setAlphaFunction(CompareFunction::Greater, 0.0f);

            if (ParticleRef::Invalid == el.get(i).iprt && ObjectRef::Invalid != el.get(i).iobj)
            {
                ObjectGraphicsRenderer::render_solid(camera, _currentModule->getObjectHandler()[el.get(i).iobj]);
            }
            else if (ObjectRef::Invalid == el.get(i).iobj && ParticleHandler::get()[el.get(i).iprt] != nullptr)
            {
                // draw draw front and back faces of polygons
                renderer.setCullingMode(CullingMode::None);

                ParticleGraphicsRenderer::render_one_prt_solid(el.get(i).iprt);
            }
        }
    }
}

} // namespace Graphics
} // namespace Ego
