#Adapted from sbrichards' doubly_linked_list.py on Githbub

from dataBaseInterface import DataBaseInterface

class Node(object):

    def __init__(self, inittime, val):
        self._data = inittime
        self.value = val
        self._next = None
        self._prev = None

    def __repr__(self):
        return "%s" % self.value

    @property
    def data(self):
        return self._data

    @property
    def next(self):
        return self._next

    @property
    def prev(self):
        return self._prev

    def set_data(self, newdata):
        self._data = newdata
        return self.data

    def set_next(self, newnext):
        self._next = newnext
        return self.next

    def set_prev(self, newprev):
        self._prev = newprev
        return self.prev


class SortedDLL(object):
    'sorted by AudioEvent.time property doubly linked list'
    def __init__(self):
        self.head = None

    def __repr__(self):
        node = self.head
        i = 0
        output = ""
        while node is not None:
            output += "[%s]=%s," % (i, node.value)
            node = node.next
            i += 1

        return output

    def isEmpty(self):
        return self.head is None

    def add(self, item):
        new = Node(item.time, item)
        if self.head is None:
            self.head = new
        elif self.head.data > new.data:
            new.set_next(self.head)
            self.head.set_prev(new)
            self.head = new
        else:
            prev = self.head
            cur = self.head.next
            while cur is not None:
                if cur.data > new.data:
                    prev.set_next(new)
                    new.set_prev(prev)
                    new.set_next(cur)
                    cur.set_prev(new)
                    return new
                prev = cur
                cur = cur.next
            prev.set_next(new)
            new.set_prev(prev)
            return new

    @property
    def size(self):
        count = 0
        cur = self.head
        while cur is not None:
            count += 1
            cur = cur.next
        return count

    def search(self, item):
        cur = self.head
        while cur is not None:
            if cur.data == item:
                return True
            elif cur.data > item:
                return False
            cur = cur.next
        return False

    def remove(self, item):
        if self.head.data == item:
            self.head = self.head.next
        else:
            prev = self.head
            cur = prev.next
            while cur is not None:
                if cur.data == item:
                    if cur.next is None:
                        prev.set_next(None)
                    else:
                        prev.set_next(cur.next)
                        cur.next.set_prev(prev)
                    return True
                prev = cur
                cur = cur.next
            return False

    def index(self, item):
        if self.search(item) == False:
            raise ValueError("Item not in list")
        index = 0
        cur = self.head
        while cur is not None:
            if cur.data == item:
                return index
            index += 1
            cur = cur.next
        return index

    def peek(self):
        if self.isEmpty():
            raise ValueError("The list is already empty")

        if self.head is not None:
            return self.head
        else:
            return None

    def pop(self, index=None):
        if self.isEmpty():
            raise ValueError("The list is already empty")
        prev = self.head
        cur = prev.next
        if index is None:
            while cur is not None:
                if cur.next is None:
                    prev.set_next(None)
                    return cur
                prev = cur
                cur = cur.next
            self.head = None
            return prev

        if index == 0:
            self.head = cur
            return prev

        cur_index = 0
        while cur is not None:
            if cur_index == index - 1:
                try:
                    prev.set_next(cur.next)
                    cur.next.set_prev(prev)
                except:
                    prev.set_next(None)
                return cur
            prev = cur
            cur = cur.next
            cur_index += 1
