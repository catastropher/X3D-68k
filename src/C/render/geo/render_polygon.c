#include "X3D_polygon.h"
#include "render/geo/X3D_render_polygon.h"
#include "render/geo/X3D_clip_polygon.h"
#include "X3D_camera.h"
#include "X3D_enginestate.h"

extern X3D_LightMapContext lightmap_context;

void x3d_render_texture_lightmap_polygon(X3D_Polygon3D* poly, X3D_Texture* tex, uint32 segment_poly_id, X3D_CameraObject* cam, X3D_LightMapContext* lightmap_context) {
    X3D_RasterPolygon3D rpoly = { .v = alloca(1000), .total_v = poly->total_v };
    
    uint16 i;
    for(i = 0; i < poly->total_v; ++i) {
        rpoly.v[i].v = poly->v[i];
        
        X3D_Vex2D v;
        x3d_planarprojection_project_point(&lightmap_context->proj[segment_poly_id], poly->v + i, &v);
        
        rpoly.v[i].uu = v.x;
        rpoly.v[i].vv = v.y;
    }
    

    X3D_PolygonRasterAtt at = {
        .surface = {
            .tex = x3d_lightmapcontext_get_surface_for_level_face(lightmap_context, segment_poly_id)
        },
        
        
        .frustum = x3d_get_view_frustum(cam)
    };
    
    X3D_ScreenManager* screenman = x3d_screenmanager_get();
    X3D_RenderManager* renderman = x3d_rendermanager_get();
    
    x3d_polygonrasteratt_set_screen(&at, x3d_screenmanager_get_screen(screenman), renderman->zbuf);
    x3d_polygon3d_render_texture_surface(&rpoly, &at, cam);
}

void x3d_render_id_buffer_polygon(X3D_Polygon3D* poly, uint32 segment_poly_id, X3D_CameraObject* cam) {
    X3D_RasterPolygon3D rpoly = { .v = alloca(1000), .total_v = poly->total_v };
    
    uint16 i;
    for(i = 0; i < poly->total_v; ++i) {
        rpoly.v[i].v = poly->v[i];
    }
    
    X3D_PolygonRasterAtt at = {
        .id_buffer = {
            .id = segment_poly_id,
        },
        .frustum = x3d_get_view_frustum(cam)
    };
    
    X3D_ScreenManager* screenman = x3d_screenmanager_get();
    X3D_RenderManager* renderman = x3d_rendermanager_get();
    
    x3d_polygonrasteratt_set_screen(&at, x3d_screenmanager_get_screen(screenman), renderman->zbuf);
    x3d_polygon3d_render_id_buffer(&rpoly, &at, cam);
}

void x3d_render_flat_shaded_polygon(X3D_Polygon3D* poly, X3D_CameraObject* cam, X3D_ColorIndex color) {
    X3D_RasterPolygon3D rpoly = { .v = alloca(1000), .total_v = poly->total_v };
    
    uint16 i;
    for(i = 0; i < poly->total_v; ++i) {
        rpoly.v[i].v = poly->v[i];
    }
    
    
    X3D_PolygonRasterAtt at = {
        .flat = {
            .color = color
        },
        
        .frustum = x3d_get_view_frustum(cam)
    };
    
    X3D_ScreenManager* screenman = x3d_screenmanager_get();
    X3D_RenderManager* renderman = x3d_rendermanager_get();
    
    x3d_polygonrasteratt_set_screen(&at, x3d_screenmanager_get_screen(screenman), renderman->zbuf);
    x3d_polygon3d_render_flat(&rpoly, &at, cam);
}

