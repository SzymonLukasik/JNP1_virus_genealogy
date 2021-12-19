#include <memory>
#include <set>
#include <map>
#include <vector>

class VirusNotFound : public std::exception {
public:
    virtual const char* what() const noexcept {return "VirusNotFound\n";}
};

class VirusAlreadyCreated : public std::exception {
public:
    virtual const char* what() const noexcept {return "VirusAlreadyCreated\n";}
};

template<typename Virus>
class VirusGenealogy {
private:

    struct VirusNode {
        std::shared_ptr<Virus> ptr;
        std::set<std::shared_ptr<Virus>> childs;
        std::set<std::shared_ptr<Virus>> parents;

        VirusNode(Virus::id_type id) : ptr(std::make_shared<Virus>(id)) {}
    };

    using nodes_t = std::map<typename Virus::id_type, std::shared_ptr<VirusNode>>;
    nodes_t nodes;
    using nodes_iterator = typename nodes_t::iterator;
    using nodes_const_iterator = typename nodes_t::const_iterator;
    typename Virus::id_type stem_id;

public:

    // Tworzy nową genealogię.
    // Tworzy także węzeł wirusa macierzystego o identyfikatorze stem_id.
    VirusGenealogy(Virus::id_type const &stem_id) : 
    nodes{{stem_id, std::make_shared<VirusNode>(stem_id)}},
    stem_id(stem_id) {}

    // Zwraca identyfikator wirusa macierzystego.
    Virus::id_type get_stem_id() const {return stem_id;}

    // Zwraca iterator pozwalający przeglądać listę identyfikatorów
    // bezpośrednich następników wirusa o podanym identyfikatorze.
    // Zgłasza wyjątek VirusNotFound, jeśli dany wirus nie istnieje.
    // Iterator musi spełniać koncept bidirectional_iterator oraz
    // typeid(*v.get_children_begin()) == typeid(const Virus &).
    //VirusGenealogy<Virus>::children_iterator get_children_begin(Virus::id_type const &id) const;

    // Iterator wskazujący na element za końcem wyżej wspomnianej listy.
    // Zgłasza wyjątek VirusNotFound, jeśli dany wirus nie istnieje.
    //VirusGenealogy<Virus>::children_iterator get_children_end(Virus::id_type const &id) const;

    // Zwraca listę identyfikatorów bezpośrednich poprzedników wirusa
    // o podanym identyfikatorze.
    // Zgłasza wyjątek VirusNotFound, jeśli dany wirus nie istnieje.
    std::vector<typename Virus::id_type> get_parents(Virus::id_type const &id) const {
        nodes_const_iterator it = nodes.find(id);
        if (it == nodes.end())
            throw (new VirusNotFound);
        std::vector<typename Virus::id_type> res;
        for (auto &it : it->second->parents) {
            res.push_back(it->get_id());
        }
        return res;
    };

    // Sprawdza, czy wirus o podanym identyfikatorze istnieje.
    bool exists(Virus::id_type const &id) const {
        return (nodes.find(id) != nodes.end());
    };

    // Zwraca referencję do obiektu reprezentującego wirus o podanym
    // identyfikatorze.
    // Zgłasza wyjątek VirusNotFound, jeśli żądany wirus nie istnieje.
    const Virus& operator[](typename Virus::id_type const &id) const {
        nodes_const_iterator it = nodes.find(id);
        if (it == nodes.end())
            throw (new VirusNotFound);
        return *(it->second->ptr);
    };

    // Tworzy węzeł reprezentujący nowy wirus o identyfikatorze id
    // powstały z wirusów o podanym identyfikatorze parent_id lub
    // podanych identyfikatorach parent_ids.
    // Zgłasza wyjątek VirusAlreadyCreated, jeśli wirus o identyfikatorze
    // id już istnieje.
    // Zgłasza wyjątek VirusNotFound, jeśli któryś z wyspecyfikowanych
    // poprzedników nie istnieje.
    void create(typename Virus::id_type const &id, typename Virus::id_type const &parent_id) {
        nodes_iterator child_it = nodes.find(id);
        if (child_it != nodes.end())
            throw (new VirusAlreadyCreated);
        nodes_iterator parent_it = nodes.find(parent_id);
        if (parent_it == nodes.end())
            throw (new VirusNotFound);
        
        VirusNode& child = *nodes.insert({id, std::make_shared<VirusNode>(id)})
                           .first->second;
        VirusNode& parent = *parent_it->second;
        parent.childs.insert(child.ptr);
        child.parents.insert(parent.ptr);
    };

    void create(typename Virus::id_type const &id, std::vector<typename Virus::id_type> const &parent_ids) {
        nodes_iterator child_it = nodes.find(id);
        if (child_it != nodes.end())
            throw (new VirusAlreadyCreated);

        std::vector<nodes_iterator> parent_its;
        for (const typename Virus::id_type &parent_id : parent_ids) {
            nodes_iterator parent_it = nodes.find(parent_id);
            if (parent_it == nodes.end())
                throw (new VirusNotFound);
            parent_its.push_back(parent_it);
        }
        VirusNode& child = *nodes.insert({id, std::make_shared<VirusNode>(id)})
                           .first->second;
        for (nodes_iterator parent_it : parent_its) {
            VirusNode& parent = *parent_it->second;
            parent.childs.insert(child.ptr);
            child.parents.insert(parent.ptr);
        }
    }; 

    // Dodaje nową krawędź w grafie genealogii.
    // Zgłasza wyjątek VirusNotFound, jeśli któryś z podanych wirusów nie istnieje.
    void connect(Virus::id_type const &child_id, Virus::id_type const &parent_id) {
        nodes_iterator child_it = nodes.find(child_id),
                                   parent_it = nodes.find(parent_id);
        
        if (child_it == nodes.end() || parent_it == nodes.end())
            throw (new VirusAlreadyCreated);
        VirusNode& child = *child_it->second, parent = *parent_it->second;
        parent.childs.insert(child.ptr);
        child.parents.insert(parent.ptr);
    };

    // Usuwa wirus o podanym identyfikatorze.
    // Zgłasza wyjątek VirusNotFound, jeśli żądany wirus nie istnieje.
    // Zgłasza wyjątek TriedToRemoveStemVirus przy próbie usunięcia
    // wirusa macierzystego.
    void remove(Virus::id_type const &id);
};