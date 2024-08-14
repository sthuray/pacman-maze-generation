#include <SFML/Graphics.hpp>
#include <random>

float toGlobalPos_x(float x) {
    float tile_dim = 8.f;
    float offset = 0.f;
    return 8.f * (x + offset);
}

float toGlobalPos_y(float y) {
    float tile_dim = 8.f;
    float offset = 3.f;
    return 8.f * (y + offset);
}

float toLocalPos_x(float x) {
    float tile_dim = 8.f;
    float offset = 0.f;
    return (x / tile_dim) - offset;
}

float toLocalPos_y(float y) {
    float tile_dim = 8.f;
    float offset = 3.f;
    return (y / tile_dim) - offset;
}

// custom vector class for ease of use
class Vec2 {
public:
    float x = 0;
    float y = 0;
    Vec2() {}
    Vec2(const float x_in, const float y_in)
        : x(x_in), y(y_in) {}
    Vec2 operator + (const Vec2& v) const {
        return Vec2(x + v.x, y + v.y);
    }
    Vec2 operator * (const float n) const {
        return Vec2(n * x, n * y);
    }
    bool operator == (const Vec2& v) const {
        return ((x == v.x) && (y == v.y));
    }
    void operator += (const Vec2& v) {
        x += v.x;
        y += v.y;
    }
    void operator -= (const Vec2& v) {
        x -= v.x;
        y -= v.y;
    }
    Vec2& add(const Vec2& v) {
        x += v.x;
        y += v.y;
        return *this;
    }
    Vec2& scale(float s) {
        x *= s;
        y *= s;
        return *this;
    }
    float dist(Vec2& v) {
        return sqrt((v.x - x)*(v.x - x) + (v.y - y)*(v.y - y));
    }

};

Vec2 randomDirection(std::vector<Vec2> possibleDirections) {
    std::random_device rd;
    std::uniform_int_distribution<int> dist(0,(possibleDirections.size() - 1)); 
    int rand = dist(rd);
    return possibleDirections[rand];
}

void resetDirections(std::vector<Vec2>& possible_directions) { 
    bool has_up = false;
    bool has_left = false;
    bool has_down = false;
    bool has_right = false;
    Vec2 up = Vec2(0, -1);
    Vec2 left = Vec2(-1, 0);
    Vec2 down = Vec2(0, 1);
    Vec2 right = Vec2(1, 0); 

    for (auto dir : possible_directions) {
        if (dir == up) {
            has_up = true;
        }
        if (dir == left) {
            has_left = true;
        }
        if (dir == down) {
            has_down = true;
        } 
        if (dir == right) {
            has_right = true;
        }
    }

    if (has_up == false) {
        possible_directions.push_back(up);
    } 
    if (has_left == false) {
        possible_directions.push_back(left);
    }
    if (has_down == false) {
        possible_directions.push_back(down);
    }
    if (has_right == false) {
        possible_directions.push_back(right);
    }
}

class CVisual {
public:
    Vec2 local_pos = {0, 0};
    Vec2 global_pos = {0, 0};
    int width;
    int height;
    sf::RectangleShape shape;
    bool has = false;
    CVisual() {} 
};

class CMovement {
public:
    std::vector<Vec2> vel_cache = {{0, 0}};
    std::vector<Vec2> possible_directions;
    bool has = false;
    CMovement() {
        resetDirections(possible_directions);
    }
};

class CBBox {
public:
    sf::FloatRect rect;
    bool has = false;
    CBBox() {}
};

class CPathTile {
public:
    std::vector<Vec2> possible_directions;
    Vec2 local_pos;
    sf::RectangleShape shape;
};

class CTile {
public:
    std::vector<Vec2> possible_directions;
    Vec2 pos;
    bool wall = false;
    float w;
    float h;
    bool has = false;
    CTile() {}
    CTile(Vec2 p, float width, float height) 
        : pos(p), w(width), h(height) {
        resetDirections(possible_directions);
    }
    CTile(float x, float y, float width, float height) 
        : pos(Vec2(x, y)), w(width), h(height) {
        resetDirections(possible_directions);
    }
};

class CDot {
public:
    Vec2 tile_pos;
    bool big;
    bool has = false;
    CDot() {}
    // TODO: create and center dot in pathtile -> mark pathtile for deletion
};

class CScore {
public:
  int points;
  bool has = false;
};

enum entityType {
    player,
    tile,
    dot, 
    enemy
};

typedef std::tuple<CVisual, CMovement, CBBox, CTile, CPathTile, CDot> ComponentTuple;

class Entity {
    friend class EntityManager;
    const entityType p_tag;
    const size_t p_id = 0;
    ComponentTuple p_components = std::make_tuple(CVisual(), CMovement(), CBBox(), CTile(), CPathTile(), CDot());
    Entity(const entityType tag, size_t id)
        : p_id(id), p_tag(tag) {}
public:
    bool p_isActive = true;
    template <typename T>
    T& getComponent() {
        return std::get<T>(p_components);
    }
    template <typename T>
    bool hasComponent() {
        return getComponent<T>().has;
    }
    template <typename T, typename... TArgs>
    T& addComponent(TArgs&&... mArgs) {
        auto& component = getComponent<T>();
        component = T(std::forward<TArgs>(mArgs)...);
        component.has = true;
        return component;
    }
    template <typename T>
    void removeComponent() {
        getComponent<T>() = T();
    }
    // assume position is in terms of local grid position
    void addPosition(float x, float y) {
        auto &p_cVis = this->getComponent<CVisual>(); 
        p_cVis.local_pos.x += x;
        p_cVis.local_pos.y += y;
        p_cVis.global_pos.x = toGlobalPos_x(p_cVis.local_pos.x);
        p_cVis.global_pos.y = toGlobalPos_y(p_cVis.local_pos.y);
        p_cVis.shape.setPosition(p_cVis.global_pos.x, p_cVis.global_pos.y);
        if (this->hasComponent<CBBox>()) {
            auto &p_cBBox = this->getComponent<CBBox>();
            p_cBBox.rect = p_cVis.shape.getGlobalBounds(); 
        }
    }
    void addPosition(Vec2 v) {
        this->addPosition(v.x, v.y);
    }
    void setPosition(float x, float y) {
        auto &p_cVis = this->getComponent<CVisual>();
        p_cVis.local_pos.x = x;
        p_cVis.local_pos.y = y;
        p_cVis.global_pos.x = toGlobalPos_x(p_cVis.local_pos.x);
        p_cVis.global_pos.y = toGlobalPos_y(p_cVis.local_pos.y);
        p_cVis.shape.setPosition(p_cVis.global_pos.x, p_cVis.global_pos.y);
        if (this->hasComponent<CBBox>()) {
            auto &p_cBBox = this->getComponent<CBBox>();
            p_cBBox.rect = p_cVis.shape.getGlobalBounds(); 
        }     
    }
};

typedef std::vector<std::shared_ptr<Entity>> EntityVec;

typedef std::map<entityType, EntityVec> EntityMap;

class EntityManager {
    EntityVec p_entities;
    EntityMap p_entityMap;
    size_t p_entityTotal = 0;
    EntityVec p_toAdd;
public:
    EntityManager() {};
    static bool toDelete(const std::shared_ptr<Entity>& e) {
        return !(e->p_isActive);
    }
    static bool toDeleteMap(const std::pair<const entityType, std::vector<std::shared_ptr<Entity>, std::allocator<std::shared_ptr<Entity>>>> e) {
        return !(e.second[0]->p_isActive);
    }
    template< typename ContainerT, typename PredicateT >
    void erase_if( ContainerT& items, const PredicateT& predicate ) {
        for( auto it = items.begin(); it != items.end(); ) {
        if( predicate(*it) ) it = items.erase(it);
        else ++it;
        }
    }
    void update() {
        for (auto e : p_toAdd) {
            p_entities.push_back(e);
            p_entityMap[e->p_tag].push_back(e);
        }
        p_toAdd.clear();

        p_entities.erase(std::remove_if(p_entities.begin(), p_entities.end(), toDelete), p_entities.end());
        for (auto& m : p_entityMap) {
            m.second.erase(std::remove_if(m.second.begin(), m.second.end(), toDelete), m.second.end());
        }
    }
    std::shared_ptr<Entity> addEntity(const entityType& tag) {
        auto e = std::shared_ptr<Entity>(new Entity(tag, p_entityTotal++));
        p_toAdd.push_back(e);
        return e;
    }
    EntityVec& getEntities() {
        return p_entities;
    }
    EntityVec& getEntities(const entityType& tag) {
        return p_entityMap[tag];
    }
};

class GameEngine {
    sf::RenderWindow p_window = sf::RenderWindow{ { 224, 290 }, "Pacman" };
    int p_fps = 144;
    float p_tiledim = 8;
    float p_w = 28;
    float p_h = 30;
    int total_score = 0;
public:
    std::array<std::shared_ptr<Entity>, (26 * 28)> p_entity_grid;
    void cacheVel(CMovement& p_cMov, float x, float y) {
        if (!((p_cMov.vel_cache[0].x == x) && (p_cMov.vel_cache[0].y == y))) {
            if (p_cMov.vel_cache.size() == 1) {
                p_cMov.vel_cache.push_back(Vec2(x, y));
            } else {
                p_cMov.vel_cache[1].x = x;
                p_cMov.vel_cache[1].y = y;
            }
        }
    }
    EntityManager EManager = EntityManager();
    GameEngine() {}
    
    // TODO: dynamic pixel movement
    void sUserInput() {
        for (auto p : EManager.getEntities(player)) {
            auto& p_cMov = p->getComponent<CMovement>();
            
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                cacheVel(p_cMov, 1.f / 16.f, 0.f);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                cacheVel(p_cMov, -1.f  / 16.f, 0.f);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
                cacheVel(p_cMov, 0.f, -1.f  / 16.f);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
                cacheVel(p_cMov, 0.f, 1.f / 16.f);
            } 
       }
    }

    bool isCollision(Vec2& vel) {
        bool collision = false;
        for (auto& p : EManager.getEntities(player)) {
            auto& p_cBBox = p->getComponent<CBBox>();
            for (auto t : EManager.getEntities(tile)) {
                if (t->hasComponent<CBBox>()) {    
                    auto& t_cBBox = t->getComponent<CBBox>();
                    if (p_cBBox.rect.intersects(t_cBBox.rect)) {
                        collision = true;
                    }
                }
            }
            for (auto& d : EManager.getEntities(dot)) {
                auto& d_cBBox = d->getComponent<CBBox>();
                if (p_cBBox.rect.intersects(d_cBBox.rect)) {
                    d->p_isActive = false;
                }
            }

            if (collision) {
                p->addPosition(vel * -1.f);
            }
        }
        return collision;
    }

    int toGridIndex(Vec2 pos) {
        auto x = pos.x - 1.f;
        auto y = pos.y - 1.f;
        auto b_w = 1.f;
        auto w = p_w - (2.f * b_w); // 26
        auto h = p_h - (2.f * b_w); // 28
        return (y * w) + x;
    }

    Vec2 fromGridIndex(int idx) {
        int b_w = 1;
        int w = p_w - (2 * b_w); // 26
        int h = p_h - (2 * b_w); // 28
        auto x = idx % w;
        if (w == 0) {
            return Vec2(-1, -1);
        }
        auto y = (idx - x) / w;
        return Vec2((float) x, (float) y);
    }

    void setInGrid(std::shared_ptr<Entity> t) {
        auto t_cTile = t->getComponent<CTile>();
        auto t_pos = t_cTile.pos;
        auto w = t_cTile.w;
        auto h = t_cTile.h;
        if (t_cTile.wall) {
            for (int y_curr = t_pos.y; y_curr < t_pos.y + h; y_curr++) {
                for (int x_curr = t_pos.x; x_curr < t_pos.x + w; x_curr++) {
                    auto index = toGridIndex(Vec2(x_curr, y_curr));
                    p_entity_grid[index] = t;
                }
            }
        } else {       
            auto index = toGridIndex(t_pos);
            p_entity_grid[index] = t;
        }  
    }

    void removeFromGrid(std::shared_ptr<Entity> t) {
        auto t_pos = t->getComponent<CTile>().pos;
        auto index = toGridIndex(t_pos);
        t->p_isActive = false;
        p_entity_grid[index] = 0;
    } 

    std::shared_ptr<Entity> getFromGrid(Vec2 pos) {
        auto index = toGridIndex(pos);
        return p_entity_grid[index];
    }
    bool isAtGrid(Vec2 pos) {
        auto index = toGridIndex(pos);
        return (p_entity_grid[index] != 0);
    }
    void horizontalIncrement(int& grid_counter) {
        int grid_w = p_w - 2.f;
        int grid_h = p_h - 2.f;

        int col_num = grid_counter % grid_w;
        int col_bottom = col_num + ((grid_h - 1) * grid_w);

        if (grid_counter == col_bottom) {
            if (col_num != grid_w - 1.f) {    
                grid_counter = col_num + 1;
            } else {
                grid_counter++;
            }
        } else {
            grid_counter += grid_w;
        }
    }
    void horizontalFill(int& grid_counter) {
        // go through entire grid array filling empty spaces with rectangles when greater than 2 tiles in a row
        // first go through horizontally, save single tile vertical spaces with an array
        // next go through created array with vertical movements
        // finally get rid of any tiles that are 1x1 tile size
        int b_w = 1.f;
        int w = (p_w - (2.f * b_w));
        int h = (p_h - (2.f * b_w));
        int grid_size = ((p_w - (2.f * b_w)) * (p_h - (2.f * b_w)));
        int seq_counter = 0;
        for (; grid_counter < grid_size; grid_counter += 1) {
            auto t_curr = p_entity_grid[grid_counter];
            if (t_curr == 0) {
                seq_counter++;
            }
            auto pos = fromGridIndex(grid_counter); 
            // row end
            if ((t_curr != 0) || ((grid_counter % w) == (w - 1.f))) {
                auto new_x = pos.x + 1.f - (seq_counter - 1);
                if (seq_counter > 1) {
                    if (t_curr != 0) {    
                        new_x -= 1; 
                    } 
                    auto f = makeWall(seq_counter, 1.f, new_x, pos.y + 1.f, true, sf::Color(210, 4, 45));
                    setInGrid(f);
                    break;
                } else {
                    seq_counter = 0;
                }
            } 
        }
        EManager.update(); 
    }
    void verticalFill(int& grid_counter) {
        int b_w = 1.f;
        int w = (p_w - (2.f * b_w));
        int h = (p_h - (2.f * b_w));
        int grid_size = ((p_w - (2.f * b_w)) * (p_h - (2.f * b_w)));
        int seq_counter = 0;
        bool empty_space = false;
        for (; grid_counter < grid_size; ) {
            auto t_curr = p_entity_grid[grid_counter]; 
            if (t_curr == 0) {
                seq_counter++;
                empty_space = true;
            } else if (t_curr->getComponent<CTile>().wall) {
                seq_counter++;
            }
            auto pos = fromGridIndex(grid_counter);
            int y = pos.y;
            if (y == (h - 1.f)) {
                y += 1.f;
            }
            if ((y == h) || ((t_curr != 0) && !(t_curr->getComponent<CTile>().wall))) {
                if ((seq_counter > 1) && (empty_space)) {
                    // blue: 0, 150, 255
                    makeWall(1.f, seq_counter, pos.x + 1.f, y - seq_counter + 1.f, true, sf::Color(210, 4, 45));
                    empty_space = false;
                    horizontalIncrement(grid_counter);
                    break;
                }
                seq_counter = 0;
                empty_space = false;
            }
            horizontalIncrement(grid_counter);
        }
        EManager.update();
    }
    void sUpdateMovement() {
        for (auto& p : EManager.getEntities(player)) {
            auto& p_cMov = p->getComponent<CMovement>();
            if (p_cMov.vel_cache.size() == 2) {
                p->addPosition(p_cMov.vel_cache[1]);
                if (isCollision(p_cMov.vel_cache[1])) {
                    p->addPosition(p_cMov.vel_cache[0]);
                    isCollision(p_cMov.vel_cache[0]);
                } else {
                    p_cMov.vel_cache.erase(p_cMov.vel_cache.begin());
                }
            } else {
                p->addPosition(p_cMov.vel_cache[0]);
                isCollision(p_cMov.vel_cache[0]);
            }
        }
    }
    void sRender() {
        p_window.setFramerateLimit(p_fps);
        float t_w = 1.f;
        float t_h = 1.f;
        float b_w = 1.f;
        EntityVec walls;

        float player_x = 3.f;
        float player_y = 14.f;
        
        auto start_tile = makeWall(t_w, t_h, player_x, player_y, false);
        setInGrid(start_tile);
        walls.push_back(start_tile); // TODO make random
        int wall_count = 0;
        EManager.update(); 
        
        bool build_wall = true;
        bool pruned = false;
        int grid_counter = 0; 
        int grid_size = ((p_w - (2.f * b_w)) * (p_h - (2.f * b_w)));
        bool h_fill = true;
        bool v_fill = true;
        bool allow_input = false;
        bool initialize_player = false;

        while (p_window.isOpen()) {
            for (auto event = sf::Event{}; p_window.pollEvent(event);) {
                if (event.type == sf::Event::Closed) {
                    p_window.close();
                }
                if (allow_input) {
                    sUserInput();
                }
            }
            sUpdateMovement();
            p_window.clear();
            // if a tile is the curr_tile after pruning, then save the current possible directions
            if (build_wall) {
                auto& t = walls[wall_count];
                auto curr_pos = t->getComponent<CTile>().pos;
                auto new_t = wallBuilder(t);
                auto new_pos = new_t->getComponent<CTile>().pos;
                if (new_pos == curr_pos) {
                    // try to prune an extra branch
                    if (toPrune(curr_pos)) {
                        // consider 2 branches in a row?
                        // consider 0 case?
                        if (wall_count != 0) {
                            wall_count--;  
                            auto& prev_t = walls[wall_count];
                            auto prev_pos = prev_t->getComponent<CTile>().pos;
                            removeFromGrid(t);
                            pruned = true;
                        } else {
                            build_wall = false;             // TODO: pruning start point moves player character for a consistent pathway system
                            pruned = false;
                        }
                    // backtrack after pruning branches
                    } else {
                        Vec2 prev_pos;
                        Vec2 n_pos;
                        do {
                            if (wall_count > 0) {
                                wall_count--;
                                if (wall_count == 0) {
                                    wall_count = wall_count + 1 - 1;
                                }
                                auto prev_t = walls[wall_count];
                                new_t = wallBuilder(prev_t);
                                auto temp_pos = prev_t->getComponent<CTile>().pos;
                                prev_pos.x = temp_pos.x;
                                prev_pos.y = temp_pos.y;
                                n_pos = new_t->getComponent<CTile>().pos;
                            } else {
                                build_wall = false;
                                break;
                            }
                        } while (n_pos == prev_pos);

                        if (pruned) {
                            pruned = false;
                        }
                    }
                } else if (pruned) {
                    pruned = false;
                }

                if ((build_wall) && (!pruned)) {
                    wall_count++;
                    auto it = walls.begin();
                    it += wall_count;
                    setInGrid(new_t);
                    walls.insert(it, new_t);
                    EManager.update();
                } else if (pruned) {
                    EManager.update();
                }
            } else if (h_fill) {
                horizontalFill(grid_counter);
                if (grid_counter == grid_size) {
                    h_fill = false;
                    grid_counter = 0;
                }
            } else if (v_fill) {
                verticalFill(grid_counter);
                if (grid_counter == grid_size) {
                    v_fill = false;
                    grid_counter = 0;
                    initialize_player = true;
                }
            // initialize player
            } else if (initialize_player) {
                
                auto p = EManager.addEntity(player);
                p->addComponent<CVisual>();
                p->addComponent<CMovement>();
                p->addComponent<CBBox>();
                auto& p_cVis = p->getComponent<CVisual>();
                auto& p_cMov = p->getComponent<CMovement>();
                auto& p_cBBox = p->getComponent<CBBox>();
                p_cVis.width = 1.f;
                p_cVis.height = 1.f;
                p_cVis.shape.setSize(sf::Vector2f(8.f, 8.f));
                p_cVis.shape.setFillColor(sf::Color(255, 219, 88));
                p->setPosition(player_x, player_y);
                EManager.update();
                // TODO: remove non-wall tiles
                for (auto t : EManager.getEntities(tile)) {
                    auto& t_cTile = t->getComponent<CTile>();
                    if (!t_cTile.wall) {
                        t->getComponent<CBBox>().has = false;
                    }
                }
                allow_input = true;
                initialize_player = false;
                EManager.update();
            }
            
            for (auto e : EManager.getEntities()) {
                if (e->hasComponent<CVisual>()) {
                    auto e_cVis = e->getComponent<CVisual>();
                    p_window.draw(e_cVis.shape);
                }
            }
            
            p_window.display();
        }
    }
    
    std::shared_ptr<Entity> makeWall(float w, float h, float x, float y, bool wall, sf::Color color = sf::Color(255, 255, 255)) {
        float tile_dim = 8.f;
        auto t = EManager.addEntity(tile);
        t->addComponent<CVisual>(); 
        t->addComponent<CTile>(x, y, w, h);
        t->addComponent<CBBox>();
        auto& t_cTile = t->getComponent<CTile>();
        auto& t_cVis = t->getComponent<CVisual>();
        auto& t_cBBox = t->getComponent<CBBox>();
        t_cVis.width = w;
        t_cVis.height = h;
        t_cVis.shape.setSize(sf::Vector2f(tile_dim * w, tile_dim * h));
        t_cVis.shape.setFillColor(color);
        t_cBBox.rect = t_cVis.shape.getGlobalBounds();
        if (wall) { 
            t_cTile.wall = true;
        }
        t->setPosition(x, y);
        return t;
    }

    void makeBorders(float w, float h, float x, float y) {
        makeWall(w, 1.f, x, y, true, sf::Color(144, 238, 144));
        makeWall(1.f, h, (w - 1), y, true, sf::Color(144, 238, 144));
        makeWall(w, 1.f, x, (h - 1), true, sf::Color(144, 238, 144));
        makeWall(1.f, h, x, y, true, sf::Color(144, 238, 144));
        EManager.update();
    }

    bool hasDoubleThickness(float new_x, float new_y) {         // can be optimised, doesn't need to check every tile
        float w = 1.f;
        float h = 1.f;
        bool u_left = false;
        bool u_mid = false;
        bool u_right = false;
        bool right = false;
        bool b_right = false;
        bool b_mid = false;
        bool b_left = false;
        bool left = false;

        // TODO use p_entity_grid instead
        for (auto t : EManager.getEntities(tile)) {
            auto t_cTile = t->getComponent<CTile>();
            if (t_cTile.wall) {
                continue;
            }
            auto t_pos = t_cTile.pos;

            if ((t_pos.x == (new_x - w))        && (t_pos.y == (new_y - h))) {
                u_left = true;
            } else if ((t_pos.x == new_x)       && (t_pos.y == (new_y - h))) {
                u_mid = true;
            } else if ((t_pos.x == (new_x + w)) && (t_pos.y == (new_y - h))) {
                u_right = true;
            } else if ((t_pos.x == (new_x + w)) && (t_pos.y == new_y)) {
                right = true;
            } else if ((t_pos.x == (new_x + w)) && (t_pos.y == (new_y + h))) {
                b_right = true;
            } else if ((t_pos.x == new_x)     && (t_pos.y == (new_y + h))) {
                b_mid = true;
            } else if ((t_pos.x == (new_x - w)) && (t_pos.y == (new_y + h))) {
                b_left = true;
            } else if ((t_pos.x == (new_x - w)) && (t_pos.y == (new_y))) {
                left = true;
            }
        } 
        
        if (left && u_left && u_mid) {
            return true; 
        } else if (u_mid && u_right && right) {
            return true;
        } else if (right && b_right && b_mid) {
            return true;
        } else if (b_mid && b_left && left) {
            return true;
        }

        if ((!left) && u_left && (!u_mid)) {
            return true;
        } else if ((!u_mid) && u_right && (!right)) {
            return true;
        } else if ((!right) && b_right && (!b_mid)) {
            return true;
        } else if ((!b_mid) && b_left && (!left)) {
            return true;
        }

        if (b_left && b_mid && u_mid && u_left) {
            return true;
        } else if (u_left && left && right && u_right) {
            return true;
        } else if (u_right && u_mid && b_mid && b_right) {
            return true;
        } else if (b_right && right && left && b_left) {
            return true;
        } else if (u_left && left && right && b_right) {
            return true;
        } else if (u_right && u_mid && b_mid && b_left) {
            return true;
        } else if (u_left && u_mid && b_mid && b_right) {
            return true;
        } else if (u_right && right && left && b_left) {
            return true;
        }

        return false;

    }

    bool isIntersecting(CBBox new_t_cBBox, float new_x, float new_y) {
        auto t_arr = EManager.getEntities(tile);
        for (auto t : t_arr) {
            auto t_rect = t->getComponent<CBBox>().rect;
            if (new_t_cBBox.rect.intersects(t_rect)) {
                return true;
            }
        }
        return false;
    }

    bool isOutOfBounds(float new_x, float new_y) {
        float w = 1.f;
        float h = 1.f;
        float uy_bound = 0.f;
        float ly_bound = 30.f;
        float rx_bound = 28.f;
        float lx_bound = 0.f;
        
        if (new_y < uy_bound) {
            return true;
        } else if ((new_y + h) > ly_bound) {
            return true;
        } else if (new_x < lx_bound) {
            return true;
        } else if ((new_x + w) > rx_bound) {
            return true;
        }
        return false;
    }

    bool isOnWall(float x, float y) {
        float w = 1.f;
        float h = 1.f;
        float b_w = 1.f;
        float uy_bound = 0.f;
        float ly_bound = 30.f;
        float rx_bound = 28.f;
        float lx_bound = 0.f;

        if (y == (uy_bound + b_w)) {
            return true;
        } else if ((x + w) == (rx_bound - b_w)) {
            return true;
        } else if ((y + h) == (ly_bound - b_w)) {
            return true;
        } else if (x == (lx_bound + b_w)) {
            return true;
        }
        return false;
    }

    bool isAlongWall(float x, float y, float new_x, float new_y) {
        if (isOnWall(x, y) && isOnWall(new_x, new_y)) {
            return true;
        }
        return false;
    }

    std::shared_ptr<Entity> wallBuilder(std::shared_ptr<Entity> t) {
        float w = 1.f;
        float h = 1.f;
        auto& t_cTile = t->getComponent<CTile>();
        auto& possible_directions = t_cTile.possible_directions;
        if (possible_directions.size() == 0) {
            return t;
        }
        float x = t_cTile.pos.x;
        float y = t_cTile.pos.y;

        Vec2 rand = randomDirection(possible_directions);
        float new_x = x + (rand.x * w);
        float new_y = y + (rand.y * h);

        auto new_t = makeWall(w, h, new_x, new_y, false);
        auto& new_t_cBBox = new_t->getComponent<CBBox>();
        auto& new_t_cTile = new_t->getComponent<CTile>();
        removeDirection(Vec2(new_x, new_y), t_cTile.pos, possible_directions);

        while (isOutOfBounds(new_x, new_y) || isIntersecting(new_t_cBBox, new_x, new_y) || hasDoubleThickness(new_x, new_y) || isAlongWall(x, y, new_x, new_y)) {
            if (possible_directions.size() == 0) {
                new_t->p_isActive = false;
                return t;
            }
            Vec2 new_rand = randomDirection(possible_directions);
            rand.x = new_rand.x;
            rand.y = new_rand.y; 
            new_x = x + (rand.x * w);
            new_y = y + (rand.y * h);
            removeDirection(Vec2(new_x, new_y), t_cTile.pos, possible_directions);
            new_t->setPosition(new_x, new_y);
        }
        new_t_cTile.pos.x = new_x;
        new_t_cTile.pos.y = new_y;
        return new_t;
    }
    
    bool toPrune(Vec2 curr_pos) {
        float w = 1.f;
        float h = 1.f;
        bool u_left = false;
        bool u_mid = false;
        bool u_right = false;
        bool right = false;
        bool b_right = false;
        bool b_mid = false;
        bool b_left = false;
        bool left = false;
        float curr_x = curr_pos.x;
        float curr_y = curr_pos.y;
 
        for (auto t : EManager.getEntities(tile)) {
            auto t_cTile = t->getComponent<CTile>();
            auto t_pos = t_cTile.pos;
            if ((t_cTile.w == w) && (t_cTile.h == h)) {
                if ((t_pos.x == (curr_x - w))        && (t_pos.y == (curr_y - h))) {
                    u_left = true;
                } else if ((t_pos.x == curr_x)       && (t_pos.y == (curr_y - h))) {
                    u_mid = true;
                } else if ((t_pos.x == (curr_x + w)) && (t_pos.y == (curr_y - h))) {
                    u_right = true;
                } else if ((t_pos.x == (curr_x + w)) && (t_pos.y == curr_y)) {
                    right = true;
                } else if ((t_pos.x == (curr_x + w)) && (t_pos.y == (curr_y + h))) {
                    b_right = true;
                } else if ((t_pos.x == curr_x)     && (t_pos.y == (curr_y + h))) {
                    b_mid = true;
                } else if ((t_pos.x == (curr_x - w)) && (t_pos.y == (curr_y + h))) {
                    b_left = true;
                } else if ((t_pos.x == (curr_x - w)) && (t_pos.y == curr_y)) {
                    left = true;
                }
            } /* else if (t_cTile.w == w) {
                if (t_pos.x == (curr_x - w)) {
                    u_left = true;
                    left = true;
                    b_left = true;
                } else if (t_pos.x == (curr_x + w)) {
                    u_right = true;
                    right = true;
                    b_right = true;
                }     
            } else if (t_cTile.h == h) {
                if (t_pos.y == (curr_y - h)) {
                    u_left = true;
                    u_mid = true;
                    u_right = true;
                } else if (t_pos.y == (curr_y + h)) {
                    b_left = true;
                    b_mid = true;
                    b_right = true;
                } 
            }*/
        }

        if (!left && !u_left && !u_mid && !u_right && !right) {
            return true;
        } else if (!u_mid && !u_right && !right && !b_right && !b_mid) {
            return true;
        } else if (!right && !b_right && !b_mid && !b_left && !left) {
            return true;
        } else if (!b_mid && !b_left && !left && !u_left && !u_mid) {
            return true;
        }

        // I shape
        if (u_left && u_mid && u_right && b_left && b_mid && b_right) {
            return true;
        } else if (u_left && left && b_left && u_right && right && b_right) {
            return true;
        }
        return false;
    }

    void removeDirection(Vec2 new_pos, Vec2 curr_pos, std::vector<Vec2>& possible_directions) {
        for (int i = 0; i < possible_directions.size(); i++) {
            if ((curr_pos + possible_directions[i]) == new_pos) {
                possible_directions.erase(possible_directions.begin() + i);
                break;    
            } 
        }
    }
};



// screen: 1680 wide, 930 tall (not including window)
// 1680.f, 120.f platform
int main() {   
    GameEngine game = GameEngine();
    game.makeBorders(28.f, 30.f, 0.f, 0.f);
    game.sRender();
}