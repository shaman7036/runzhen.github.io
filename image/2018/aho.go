package trie

import "fmt"

type Node struct {
	val    rune
	termi  bool
	depth  int
	meta   interface{}
	parent *Node
	child  map[rune]*Node
	fail   *Node
	saved  []string
}

type Trie struct {
	root *Node
	size int
}

// Create a new trie with an initialized root node
func New() *Trie {
	return &Trie{
		root: &Node{
			child: make(map[rune]*Node),
			depth: 0,
			fail:  nil},
		size: 0,
	}
}

// return trie root
func (t *Trie) Root() *Node {
	return t.root
}

// Add the key to the trie.
func (t *Trie) Add(key string, meta interface{}) *Node {
	t.size++
	runes := []rune(key)

	node := t.root

	for _, r := range runes {
		if n, ok := node.child[r]; ok {
			node = n
		} else {
			node = node.NewChild(r, nil, false)
		}
	}
	node.termi = true
	node.saved = append(node.saved, key)
	//fmt.Printf("add %s \n", key)
	node.meta = meta

	return node
}

func (n *Node) NewChild(val rune, meta interface{}, termi bool) *Node {
	node := &Node{
		val:    val,
		termi:  termi,
		meta:   meta,
		parent: n,
		child:  make(map[rune]*Node),
		depth:  n.depth + 1,
		fail:   nil,
		saved:  make([]string, 0),
	}
	n.child[val] = node

	return node
}

func (n *Node) Children() map[rune]*Node {
	return n.child
}

// AhoBuilder
func (t *Trie) AhoBuilder() {
	queue := make([]*Node, 0)
	queue = append(queue, t.Root())

	var queueHead *Node
	var p *Node

	for {
		if len(queue) == 0 {
			break
		}

		// at the beginning, put the Root to queue
		// root's fail = nil
		queueHead = queue[0]

		// iterate current Node's children
		for r, n := range queueHead.Children() {

			p = queueHead.fail

			// root's children fail pointer points to root
			if queueHead == t.Root() {
				n.fail = t.Root()
			} else {

				for {
					if p != nil && p.Children()[r] == nil {
						p = p.fail
					} else {
						break
					}
				}

				if p != nil {
					n.fail = p.Children()[r]
				} else {
					n.fail = t.Root()
				}
			}

			queue = append(queue, n)
		}
		queue = queue[1:]
	}

}

func printSaved(n *Node, i int, ch string) {
	fmt.Printf("Node %p hit at str[%d] \"%s\": ", n, i, ch)
	for _, k := range n.saved {
		fmt.Printf("%s, ", k)
	}
	fmt.Printf("\n")
}

// Finds the key
func (t *Trie) Find(key string) bool {

	str := []rune(key)

	p := t.Root()
	var ch rune

	for i := 0; i < len(str); i++ {
		ch = str[i]

		for {
			if p.Children()[ch] == nil && p != t.Root() {
				p = p.fail
			} else {
				break
			}
		}

		p = p.Children()[ch]

		if p == nil {
			p = t.Root()
		} else {
			if p.termi == true {
				printSaved(p, i, string(ch))
				// 匹配该节点后,沿其失败指针回溯,判断其他节点是否匹配
				temp := p
				for {
					temp = temp.fail

					if temp == nil {
						break
					}

					if temp.termi == true {
						printSaved(temp, i, string(""))
					}
				}
			}
		}
	}
	return true
}
