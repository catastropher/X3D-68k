// This file is part of X3D.
//
// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#include <new>

#include "EntityModel.hpp"
#include "EntityModelLoader.hpp"
#include "error/Log.hpp"
#include "error/Error.hpp"

#include "render/RenderContext.hpp"
#include "geo/Ray3.hpp"
#include "geo/Polygon3.hpp"
#include "math/Mat4x4.hpp"
#include "render/Camera.hpp"
#include "render/AffineTriangleFiller.hpp"

bool x_entitymodel_load_from_file(X_EntityModel* model, const char* fileName)
{
    X_EntityModelLoader loader;
    
    if(!x_entitymodelloader_load_model_from_file(&loader, fileName, model))
    {
        x_log_error("Failed to load model %s\n", fileName);
        return 0;
    }
    
    x_log("Loaded model %s", model->name);
    
    return 1;
}

void x_entitymodel_get_skin_texture(X_EntityModel* model, int skinId, int textureId, Texture* dest)
{
    x_assert(skinId < model->totalSkins && textureId < model->skins[skinId].totalTextures, "Bad skin/texture id");

    new (dest) Texture(model->skinWidth, model->skinHeight, model->skins[skinId].textures[textureId].texels);
}

void x_entitymodel_cleanup(X_EntityModel* model)
{
    // Each skin allocates its textures together, so just free the first
    for(int i = 0; i < model->totalSkins; ++i)
    {
        x_free(model->skins[i].textures[0].texels);
        x_free(model->skins[i].textures);
    }
    
    x_free(model->skins);
    x_free(model->textureCoords);
    x_free(model->triangles);
    
    for(int i = 0; i < model->totalFrameGroups; ++i)
    {
        X_EntityFrameGroup* group = model->frameGroups + i;
        for(int frameId = 0; frameId < group->totalFrames; ++frameId)
        {
            X_EntityFrame* frame = group->frames + frameId;
            x_free(frame->vertices);
        }
        
        x_free(group->frames);
    }
    
    x_free(model->frameGroups);
}

X_EntityFrame* x_entitymodel_get_frame(X_EntityModel* model, const char* frameName)
{
    for(int i = 0; i < model->totalFrameGroups; ++i)
    {
        X_EntityFrameGroup* group = model->frameGroups + i;
        
        for(int frameId = 0; frameId < group->totalFrames; ++frameId)
        {
            if(strcmp(group->frames[frameId].name, frameName) == 0)
                return group->frames + frameId;
        }
    }
    
    return NULL;
}

X_EntityFrame* x_entitymodel_get_animation_start_frame(X_EntityModel* model, const char* animationName)
{
    char startFrameName[20];
    sprintf(startFrameName, "%s1", animationName);
    return x_entitymodel_get_frame(model, startFrameName);
}

void x_entitymodel_draw_frame_wireframe(X_EntityModel* model, X_EntityFrame* frame, Vec3 pos, X_Color color, X_RenderContext* renderContext)
{
    for(X_EntityTriangle* triangle = model->triangles; triangle < model->triangles + model->totalTriangles; ++triangle)
    {
        Vec3fp v[3];
        X_EntityVertex* vertices[3];
        
        for(int i = 0; i < 3; ++i)
        {
            vertices[i] = frame->vertices + triangle->vertexIds[i];
            v[i] = MakeVec3fp(vertices[i]->v) + MakeVec3fp(pos);
        }
        
        for(int i = 0; i < 3; ++i)
        {
            Ray3 ray(v[i], v[(i + 1) % 3]);
            ray.render(*renderContext, color);
        }
    }
}

struct ModelVertex
{
    int x;
    int y;
    int s;
    int t;
    int z;

    void print(const char* name) const
    {
        printf("%s: x=%d, y=%d, z=%d, s=%d, t=%d\n", name, x, y, z, s, t);
    }
};

void drawTriangleRecursive(
    const ModelVertex* a,
    const ModelVertex* b,
    const ModelVertex* c,
    const Texture& skin,
    X_RenderContext& renderContext);

void x_polygon3_render_textured(Polygon3* poly, X_RenderContext* renderContext, Texture* texture, Vec2i textureCoords[3])
{
     Vec3fp clippedV[X_POLYGON3_MAX_VERTS];
     Polygon3 clipped(clippedV, X_POLYGON3_MAX_VERTS);

     if(!poly->clipToFrustum(*renderContext->viewFrustum, clipped, (1 << 4) - 1))
     {
         return;
     }

     if(clipped.totalVertices != 3)
     {
         return;
     }

     X_TriangleFiller filler;
     x_trianglefiller_init(&filler, renderContext);

     for(int i = 0; i < 3; ++i)
     {
         Vec3fp transformed;
         transformed = renderContext->viewMatrix->transform(clipped.vertices[i]);

         Vec2_fp16x16 projected;
         renderContext->cam->viewport.project(transformed, projected);

         Vec2i coord(projected.x >> 16, projected.y >> 16);

         x_trianglefiller_set_textured_vertex(&filler, i, coord, transformed.z.toInt(), textureCoords[i]);
     }

 //     Plane plane;
 //     x_plane_init_from_three_points(&plane, poly->vertices + 0, poly->vertices + 1, poly->vertices + 2);
 //
 //     if(!x_plane_point_is_on_normal_facing_side(&plane, &renderContext->camPos))
 //         return;
 //
     x_trianglefiller_fill_textured(&filler, texture);
}

void x_entitymodel_render_flat_shaded(X_EntityModel* model, X_EntityFrame* frame, Mat4x4& transformMatrix, X_RenderContext* renderContext)
{
    Texture skin;
    x_entitymodel_get_skin_texture(model, 0, 0, &skin);

    for(int i = 0; i < model->totalTriangles; ++i)
    {
        Vec3fp v[3];
        X_EntityTriangle* tri = model->triangles + i;
        Vec2 textureCoords[3];
        ModelVertex modelVertex[3];

        Vec2_fp16x16 projected[3];
        
        for(int j = 0; j < 3; ++j)
        {
            v[j] = MakeVec3fp(frame->vertices[tri->vertexIds[j]].v);

            Vec3fp transformed = transformMatrix.transform(v[j]);

            v[j] = transformed;


            renderContext->cam->viewport.project(transformed, projected[j]);

            renderContext->cam->viewport.clampfp(projected[j]);

            Vec2i pos(projected[j].x >> 16, projected[j].y >> 16);

            X_EntityTextureCoord* coord = model->textureCoords + tri->vertexIds[j];
            
            textureCoords[j] = coord->coord;
            if(!tri->facesFront && coord->onSeam)
            {
                textureCoords[j].x += model->skinWidth / 2;
            }

            X_Color texel = skin.getTexel(textureCoords[j]);

            //renderContext->canvas->setTexel(pos, texel);

            modelVertex[j].x = pos.x;
            modelVertex[j].y = pos.y;
            modelVertex[j].z = (1.0_fp / transformed.z).internalValue();
            modelVertex[j].s = textureCoords[j].x;
            modelVertex[j].t = textureCoords[j].y;
        }

        Polygon3 poly(v, 3);

        x_polygon3_render_textured(&poly, renderContext, &skin, textureCoords);

        //drawTriangleRecursive(&modelVertex[0], &modelVertex[1], &modelVertex[2], skin, *renderContext);
    }
}

static void splitEdge(const ModelVertex& a, const ModelVertex& b, ModelVertex& outVertex)
{
    outVertex.x = (a.x + b.x) / 2;
    outVertex.y = (a.y + b.y) / 2;
    outVertex.s = (a.s + b.s) / 2;
    outVertex.t = (a.t + b.t) / 2;
    outVertex.z = (a.z + b.z) / 2;
}

static bool edgeHasAtLeastOnePixel(const ModelVertex& a, const ModelVertex& b)
{
    int xLength = a.x - b.x;
    if(xLength < -1 || xLength > 1)
    {
        return true;
    }

    int yLength = a.y - b.y;
    if(yLength < -1 || yLength > 1)
    {
        return true;
    }

    return false;
}

void drawTriangleRecursive(
    const ModelVertex* a,
    const ModelVertex* b,
    const ModelVertex* c,
    const Texture& skin,
    X_RenderContext& renderContext)
{
    ModelVertex newVertex;

    const ModelVertex* splitStart;
    const ModelVertex* splitEnd;
    const ModelVertex* other;

    if(edgeHasAtLeastOnePixel(*a, *b))
    {
        splitStart = a;
        splitEnd = b;
        other = c;
    }
    else if(edgeHasAtLeastOnePixel(*b, *c))
    {
        splitStart = b;
        splitEnd = c;
        other = a;
    }
    else if(edgeHasAtLeastOnePixel(*c, *a))
    {
        splitStart = c;
        splitEnd = a;
        other = b;
    }
    else
    {
        return;
    }

    splitEdge(*splitStart, *splitEnd, newVertex);


    if(splitEnd->y - splitStart->y > 0)
    {
        int texelIndex = a->y * renderContext.screen->getW() + a->x;
        x_fp0x16& zbuf = renderContext.zbuf[texelIndex];

        //if(a.z < zbuf)
        //{
        zbuf = a->z;

        int s = clamp(a->s, 0, 639);
        int t = clamp(a->t, 0, 379);

        X_Color color = skin.getTexel({s, t});
        renderContext.screen->canvas.setTexel({a->x, a->y}, color);
        //}
    }

    drawTriangleRecursive(splitStart, other, &newVertex, skin, renderContext);
    drawTriangleRecursive(other, &newVertex, splitEnd, skin, renderContext);
}