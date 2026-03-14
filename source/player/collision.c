#include "collision.h"
#include "player.h"
#include <math.h>
#include "state.h"
#include <citro3d.h>
#include "graphics.h"
#include "text.h"
#include "fonts/bigFont.h"
#include "menus/components/ui_screen.h"

void get_corners(float cx, float cy, float w, float h, float angle, Vec2D out[4]) {
    float hw = w / 2.0f, hh = h / 2.0f;
    angle = -angle; // Make it clockwise
    float rad = C3D_AngleFromDegrees(angle);
    float cos_a = cosf(rad), sin_a = sinf(rad);
    

    float local[4][2] = {
        { -hw, -hh },
        {  hw, -hh },
        {  hw,  hh },
        { -hw,  hh }
    };
    for (int i = 0; i < 4; ++i) {
        out[i].x = cx + local[i][0] * cos_a - local[i][1] * sin_a;
        out[i].y = cy + local[i][0] * sin_a + local[i][1] * cos_a;
    }
}

static bool sat_overlap(const Vec2D a[4], const Vec2D b[4]) {
    // Test all axes (normals of edges)
    for (int shape = 0; shape < 2; ++shape) {
        const Vec2D *verts = (shape == 0) ? a : b;
        for (int i = 0; i < 4; ++i) {
            // Edge from verts[i] to verts[(i+1)%4]
            float dx = verts[(i+1)%4].x - verts[i].x;
            float dy = verts[(i+1)%4].y - verts[i].y;
            // Normal axis
            float ax = -dy, ay = dx;

            // Project both shapes onto axis
            float minA = INFINITY, maxA = -INFINITY;
            float minB = INFINITY, maxB = -INFINITY;
            for (int j = 0; j < 4; ++j) {
                float projA = a[j].x * ax + a[j].y * ay;
                float projB = b[j].x * ax + b[j].y * ay;
                if (projA < minA) minA = projA;
                if (projA > maxA) maxA = projA;
                if (projB < minB) minB = projB;
                if (projB > maxB) maxB = projB;
            }
            // If projections do not overlap, there is a separating axis
            if (maxA <= minB || maxB <= minA) return false;
        }
    }
    return true;
}

bool intersect(float x1, float y1, float w1, float h1, float angle1,
               float x2, float y2, float w2, float h2, float angle2) {
    float big = fmaxf(w1, h1) + fmaxf(w2, h2);
    if (fabsf(x1 - x2) > big || fabsf(y1 - y2) > big) {
        return false;
    }
    
    Vec2D rect1[4], rect2[4];
    get_corners(x1, y1, w1, h1, angle1, rect1);
    get_corners(x2, y2, w2, h2, angle2, rect2);
    return sat_overlap(rect1, rect2);
}

bool intersect_rect_circle(float rx, float ry, float rw, float rh, float rangle,
                          float cx, float cy, float cradius) {
    // If centers are too far apart, no collision
    float max_dim = fmaxf(rw, rh);
    float max_dist = (max_dim / 2.0f) + cradius;
    if (fabsf(rx - cx) > max_dist || fabsf(ry - cy) > max_dist) {
        return false;
    }

    // Transform circle center into rectangle's local space
    float rad = -C3D_AngleFromDegrees(rangle); // negative for inverse rotation
    float cos_a = cosf(rad), sin_a = sinf(rad);

    float local_cx = cos_a * (cx - rx) - sin_a * (cy - ry) + rx;
    float local_cy = sin_a * (cx - rx) + cos_a * (cy - ry) + ry;

    // Rectangle bounds
    float left   = rx - rw / 2.0f;
    float right  = rx + rw / 2.0f;
    float top    = ry - rh / 2.0f;
    float bottom = ry + rh / 2.0f;

    // Find closest point on rectangle to circle center
    float closest_x = fmaxf(left, fminf(local_cx, right));
    float closest_y = fmaxf(top,  fminf(local_cy, bottom));

    // Distance from circle center to closest point
    float dx = local_cx - closest_x;
    float dy = local_cy - closest_y;
    float dist_sq = dx * dx + dy * dy;

    return dist_sq <= cradius * cradius;
}
// Dot product helper
float dot(float x1, float y1, float x2, float y2) {
    return x1 * x2 + y1 * y2;
}

bool circle_rect_collision(float cx, float cy, float radius,
                           float x1, float y1, float x2, float y2) {
    // Vector from point 1 to circle center
    float dx = x2 - x1;
    float dy = y2 - y1;
    float fx = cx - x1;
    float fy = cy - y1;

    float len_sq = dx * dx + dy * dy;
    float t = dot(fx, fy, dx, dy) / len_sq;

    // Clamp t to the [0,1] range to stay within the segment
    if (t < 0.0f) t = 0.0f;
    else if (t > 1.0f) t = 1.0f;

    // Closest point on segment
    float closestX = x1 + t * dx;
    float closestY = y1 + t * dy;

    // Distance from circle center to closest point
    float distX = cx - closestX;
    float distY = cy - closestY;
    float distSq = distX * distX + distY * distY;

    return distSq <= radius * radius;
}

void handle_collision(Player *player, int obj, const ObjectHitbox *hitbox) {
    InternalHitbox internal = player->internal_hitbox;

    int clip = (player->gamemode == GAMEMODE_SHIP || player->gamemode == GAMEMODE_BIRD) ? 7 : 10;
    switch (hitbox->collision_type) {
        //case HITBOX_BREAKABLE_BLOCK:
        case HITBOX_SOLID: 
            bool gravSnap = false;

            clip += fabsf(player->vel_y) * STEPS_DT;
            /*
            float bottom = gravBottom(player);
            if (player->slope_data.slope) {
                // Something that makes the slope not reset the speed
                bottom = bottom + sinf(slope_angle(player->slope_data.slope, player)) * player->height / 2;
                clip = 7;
                if (obj_gravTop(player, obj) - bottom < 2)
                    return;
            }
            
            // Collide with slope if object is an slope
            if (objects[*soa_id(obj)].is_slope) {
                slope_collide(obj, player);
                break;
            }*/
            /*
            if (player->gravObj_id && player->gravObj->hitbox_counter[state.current_player] == 1) {
                // Only do the funny grav snap if player is touching a gravity object and internal hitbox is touching block
                bool internalCollidingBlock = intersect(
                    player->x, player->y, internal.width, internal.height, 0, 
                    *soa_x(obj), *soa_y(obj), hitbox->width, hitbox->height, obj->rotation
                );

                gravSnap = (!state.old_player.on_ground || player->ceiling_inv_time > 0) && internalCollidingBlock && obj_gravTop(player, obj) - gravInternalBottom(player) <= clip;
            }
            */
            /*
            bool slope_height_check = false;
            if (player->touching_slope) {
                if (grav_slope_orient(player->potentialSlope, player) == ORIENT_NORMAL_DOWN) {
                    slope_height_check = gravBottom(player) < grav(player, *soa_y(player->potentialSlope));
                } else if (grav_slope_orient(player->potentialSlope, player) == ORIENT_UD_DOWN) {
                    slope_height_check = gravTop(player) > grav(player, *soa_y(player->potentialSlope));
                }
            }
            bool slope_condition = player->touching_slope && !slope_touching(player->potentialSlope, player) && slope_height_check && (player->potentialSlope->object.orientation == ORIENT_NORMAL_DOWN || player->potentialSlope->object.orientation == ORIENT_UD_DOWN);

            // Snap the player to the potential slope when the player is touching the slope
            if (player->touching_slope && slope_touching(player->potentialSlope, player) && slope_height_check) {
                slope_collide(player->potentialSlope, player);
                break;
            }*/
            
            bool safeZone = player->mini && ((obj_gravTop(player, obj) - gravBottom(player) <= clip) || (gravTop(player) - obj_gravBottom(player, obj) <= clip));
            
            if ((player->gamemode == GAMEMODE_DART || (!gravSnap && !safeZone)) && intersect(
                player->x, player->y, internal.width, internal.height, 0, 
                objects.x[obj], objects.y[obj], hitbox->width, hitbox->height, objects.rotation[obj]
            )) {
                /*if (hitbox->type == HITBOX_BREAKABLE_BLOCK) {
                    // Spawn breakable brick particles
                    obj->hide_sprite = TRUE;
                    for (s32 i = 0; i < 10; i++) {
                        spawn_particle(BREAKABLE_BRICK_PARTICLES, *soa_x(obj), *soa_y(obj), obj);
                    }
                } else {*/
                    // Not a brick, die
                    state.dead = true;
                //}
            // Check snap for player bottom
            } else if (obj_gravTop(player, obj) - gravBottom(player) <= clip && player->vel_y <= 0 &&/* !slope_condition && */player->gamemode != GAMEMODE_DART) {
                player->y = grav(player, obj_gravTop(player, obj)) + grav(player, player->height / 2);
                if (player->vel_y <= 0) player->vel_y = 0;
                player->on_ground = true;
                player->inverse_rotation = false;
                player->time_since_ground = 0;
            // Check snap for player top
            } else if (player->gamemode != GAMEMODE_DART) {
                // Ufo can break breakable blocks from above, so dont use as a ceiling
                if (player->gamemode == GAMEMODE_BIRD/* && hitbox->type == HITBOX_BREAKABLE_BLOCK*/) {
                    break;
                }
                // Behave normally
                if (player->gamemode != GAMEMODE_PLAYER || gravSnap) {
                    if (((gravTop(player) - obj_gravBottom(player, obj) <= clip || gravSnap))) {// && !slope_condition) {
                        if (!gravSnap) player->on_ceiling = true;
                        player->inverse_rotation = false;
                        player->time_since_ground = 0;
                        player->ceiling_inv_time = 0;
                        player->y = grav(player, obj_gravBottom(player, obj)) - grav(player, player->height / 2);
                        if (player->vel_y >= 0) player->vel_y = 0;
                    }
                }
            }
            break;
        case HITBOX_HAZARD:
            state.dead = true;
            break;
        case HITBOX_SPECIAL:
            //handle_special_hitbox(player, obj, hitbox);
            break;
    }
}

void collide_with_obj(Player *player, int obj) {
    int obj_id = objects.id[obj];
    const ObjectHitbox *hitbox = game_objects[obj_id].hitbox;

    if (!hitbox) return;

    //number_of_collisions_checks++;

    float x = objects.x[obj];
    float y = objects.y[obj];
    float width = objects.width[obj];
    float height = objects.height[obj];

    if (hitbox->type == COLLISION_CIRCLE) {
        if (intersect_rect_circle(
            player->x, player->y, player->width, player->height, player->rotation, 
            x, y, hitbox->width
        )) {
            handle_collision(player, obj, hitbox);
            //obj->collided[state.current_player] = true;
            //number_of_collisions++;
        } else {
            //obj->collided[state.current_player] = false;
        }
    } else {
        float obj_rot = normalize_angle(objects.rotation[obj]);

        if (hitbox->collision_type == HITBOX_SOLID) {
            obj_rot = 0;
        }

        float rotation = (obj_rot == 0 || obj_rot == 90 || obj_rot == 180 || obj_rot == 270) ? 0 : player->rotation;
        
        bool checkColl = intersect(
            player->x, player->y, player->width, player->height, rotation, 
            x, y, width, height, obj_rot
            
        );

        // Rotated hitboxes must also collide with the unrotated hitbox
        if (rotation != 0) {
            checkColl = checkColl && intersect(
                player->x, player->y, player->width, player->height, 0, 
                x, y, width, height, obj_rot
            );
        }

        if (checkColl) {
            handle_collision(player, obj, hitbox);
            //obj->collided[state.current_player] = true;
            //number_of_collisions++;
        } else {
            //obj->collided[state.current_player] = false;
        }
    }
}

int slope_buffer[MAX_COLLIDED_OBJECTS];
int slope_count = 0;

int block_buffer[MAX_COLLIDED_OBJECTS];
int block_count = 0;

int hazard_buffer[MAX_COLLIDED_OBJECTS];
int hazard_count = 0;

void collide_with_objects(Player *player) {
    //number_of_collisions = 0;
    //number_of_collisions_checks = 0;

    int sx = (int)(player->x / SECTION_SIZE);
    
    for (int dx = -1; dx <= 1; dx++) {
        Section *sec = get_or_create_section(sx + dx);
        for (int i = 0; i < sec->object_count; i++) {
            int obj = sec->objects[i];
            const ObjectHitbox *hitbox = game_objects[objects.id[obj]].hitbox;

            if (!hitbox) continue;
            
            // Save some types to buffer, so they can be checked in a type order
            if (hitbox->collision_type == HITBOX_SOLID) {
                /*if (objects[*soa_id(obj)].is_slope) {
                    slope_buffer[slope_count++] = obj;
                } else {
                    block_buffer[block_count++] = obj;
                }*/
                block_buffer[block_count++] = obj;
            } else if (hitbox->collision_type == HITBOX_HAZARD) {
                hazard_buffer[hazard_count++] = obj;
            } else { // HITBOX_SPECIAL
                collide_with_obj(player, obj);
            }
        }
    }

    //if (player->left_ground) {
    //    clear_slope_data(player);
    //}

    /*float closestDist = 999999.f;
    // Detect if touching slope
    for (int i = 0; i < slope_count; i++) {
        int obj = slope_buffer[i];
        if (intersect(
            player->x, player->y, player->width, player->height, 0, 
            objects.x[obj], objects.y[obj], objects.width[obj], objects.height[obj], objects.rotation[obj]
        )) {
            float dist = fabsf(objects.y[obj] - player->y);
            if (dist < closestDist) {
                player->touching_slope = true;
                player->potentialSlope_id = obj;
                closestDist = dist; 
            }
        }
    }*/

    for (int i = 0; i < block_count; i++) {
        int obj = block_buffer[i];
        collide_with_obj(player, obj);
    }

    /*bool has_slope = player->slope_data.slope_id;
    for (int i = 0; i < slope_count; i++) {
        GameObject *obj = slope_buffer[i];
        collide_with_slope(player, obj, has_slope);
    }*/
    
    for (int i = 0; i < hazard_count; i++) {
        int obj = hazard_buffer[i];
        collide_with_obj(player, obj);
    }

    player->touching_slope = false;
    slope_count = 0;
    block_count = 0;
    hazard_count = 0;
}