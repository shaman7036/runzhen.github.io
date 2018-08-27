---
layout: post
comments: no
title: 二叉树的遍历
category:
tags: 
---


## 二叉树的遍历

1. 前序遍历：根节点->左孩子->右孩子
2. 中序遍历：左孩子->根节点->右孩子
3. 后序遍历：左孩子->右孩子->根节点

## 递归遍历

```c
    void preorder(TreeNode *root, vector<int> &result) {
        if (root == NULL)
            return;

        result.push_back(root->val);
        preorder(root->left, result);
        preorder(root->right, result);
    }
```

```c
    void inorder(TreeNode *root, vector<int> &result) {
        if (root == NULL) {
            return;
        }
        inorder(root->left, result);
        result.push_back(root->val);
        inorder(root->right, result);
    }
```

```c
    void postorder(TreeNode *root, vector<int> &result) {
        if (root == NULL) 
            return ;

        postorder(root->left, result);
        postorder(root->right, result);
        result.push_back(root->val);
    }
```

## 非递归遍历

二叉树的 前序、中序、后序遍历都只用到栈，不会用到 队列。

而 BFS 遍历的话，则会用到队列。

### 前序非递归遍历

`向左走到底` 法。


```c
class Solution {
    public:
        vector<int> preorderTraversal(TreeNode* root) {
            vector<int> result;
            stack<TreeNode *> stack;

            if (root == NULL)
                return result;

            TreeNode * node = root;

            while (!stack.empty() || node) {
            	// 一路向左走到底。
                while (node) {
                	// 在这里访问值
                    result.push_back(node->val);
                    stack.push(node);
                    node = node->left;
                }

                if (!stack.empty()) {
                    node = stack.top();
                    stack.pop();
                    // 对右子树 准备也执行 一左到底
                    node = node->right;
                }
            }

            return result;
        }
};
```
### 中序非递归遍历

中序遍历的代码结构几乎与前序遍历的一模一样，唯一的不同是访问 val 的地方，前序是在 `一左到底` 的地方，中序是在跳到右子树之前。

```c
class Solution {
public:
    vector<int> inorderTraversal(TreeNode *root) {
        stack<TreeNode*> stack;
        vector<int> result;
        if (root == NULL)
            return result;

        TreeNode *node = root;

        while (!stack.empty() || node) {
        	// 一左到底
            while (node) {
                stack.push(node);
                node = node->left;
            }

            if (!stack.empty()) {
                node = stack.top();
                stack.pop();

                result.push_back(node->val);
                node = node->right;
            }
        }

        return result;
    }
};
```

### 后序非递归遍历 

第一种方法代码整体结构看上去和前序、中序大体一致，但是遵循的的是 `一路向右走到底` 的策略，而且需要一个额外的 stack 保存每个节点的 val。因此会多用些内存，但好处是容易记忆。

```c
class Solution {
    public:
        vector<int> postorderTraversal(TreeNode* root) {
            vector<int> result;
            stack<int> val_stack;
            stack<TreeNode *> stack;

            if (root == NULL)
                return result;

            TreeNode *node = root;

            while (!stack.empty() || node) {

            	// 对每个节点一路向右走到底
                while (node) {
                    stack.push(node);
                    // 保存值到栈中，而不是直接放入 result 数组
                    val_stack.push(node->val);
                    node = node->right; 
                }

                // 每出栈一个节点，取其左孩子
                if (!stack.empty()) {
                    node = stack.top();
                    stack.pop();
                    node = node->left;
                }
            } // traversal end;

            // pop the val stack
            while (!val_stack.empty()) {
                result.push_back(val_stack.top());
                val_stack.pop();
            }

            return result;
        }
};
```

第二种方法，代码结构与前序、中序略有差异，但是逻辑上保持一致，即仍然是 `一路向左走到底`，但是需要多用一个指针，具体看代码注释。

```c
class Solution {
    public:
        vector<int> postorderTraversal(TreeNode* root) {
            vector<int> result;
            stack<TreeNode *> stack;
            TreeNode *node = root, *preVisit = NULL;

            if (root == NULL)
                return result;

            // 先 `一左到底`
            while (node) {
                stack.push(node);
                node = node->left;
            }

            while (!stack.empty()) {

                node = stack.top();
                stack.pop();
                // 一个根节点被访问的前提是：无右子树或右子树已被访问过
                if (node->right == NULL || node->right == preVisit) {
                    result.push_back(node->val);
                    preVisit = node;
                } else {
                    // node->left == preVisit
                    // 若左子树刚被访问过，则需先进入右子树(根节点需再次入栈)
                    stack.push(node);
                    node = node->right;
                    // 只有在 node 跳到 右子树时 才能有 `一左到底`
                    // 这就是为什么 有两个这样的 while 循环，
                    // 也是为什么后序的代码结构与前序,中序不一样
                    while (node) {
                        stack.push(node);
                        node = node->left;
                    }
                }
            }
            return result;
        }
};

```

## 多叉树的遍历

本以为多叉树的遍历会更加麻烦，结果却发现貌似更加简单，因为前序和后序的代码结构大体相同。

### 多叉树前序遍历

```
class Solution {
public:
    vector<int> preorder(Node* root) {
        vector<int> result;
        stack<Node*> stk;
        
        if (root == NULL) 
            return result;
        
        Node * nd = root;
        stk.push(nd);
        
        while (!stk.empty()) {
            nd = stk.top();
            stk.pop();
            result.push_back(nd->val);
            
            // use rbegin() rend()
            for (auto ite = nd->children.rbegin(); ite != nd->children.rend(); ++ite) {
                stk.push(*ite);
            }
        }
        return result;
        
    }
};
```

### 多叉树后序遍历
```
class Solution {
public:
    vector<int> postorder(Node* root) {
        vector<int> result;
        stack<Node *> stk;

        if (root == NULL)
            return result;

        Node *nd = root;
        stk.push(nd);

        while (!stk.empty()) {
            nd = stk.top();
            stk.pop();

            result.push_back(nd->val);

            for (auto it = nd->children.begin(); it != nd->children.end(); ++it) {
                stk.push(*it);
            }
        }
        
        // reverse is important!!!!
        reverse(result.begin(), result.end());
        return result;
    }
};
```

### 递归中嵌套递归

比如题目 [Path Sum III](https://leetcode.com/problems/path-sum-iii/description/)，如果 sum = 8，那么子树中有三个符合条件。

```
      10
     /  \
    5   -3
   / \    \
  3   2   11
 / \   \
3  -2   1
```

解题思路：以 10 为 root，DFS 遍历一次；然后分别以其他子节点为 root，分别 DFS，所以这是两个 DFS 的嵌套。

```
class Solution {
public:
    int pathSum(TreeNode* root, int sum) {
        int count = 0;

        dfs(root, sum, count);

        if (root) {
            count += pathSum(root->left, sum) +
                     pathSum(root->right, sum);
        }
        return count;
    }

    void dfs(TreeNode *root, int sum, int& count) {
        if (root == NULL)
            return ;
        if (sum == root->val) {
            count++;
        }
        dfs(root->left, sum-root->val, count);
        dfs(root->right, sum-root->val, count);
    }
};
```

### 参考资料
- [https://blog.csdn.net/zhangxiangDavaid/article/details/37115355](https://blog.csdn.net/zhangxiangDavaid/article/details/37115355)
- [http://kubicode.me/2015/03/24/Data%20Struct/bitTree-traversal/](http://kubicode.me/2015/03/24/Data%20Struct/bitTree-traversal/)


