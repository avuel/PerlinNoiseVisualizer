#pragma once

#include <vector>
#include <memory>

#include "AABB.hpp"

class OcTree
{
public:
	OcTree() = default;
	~OcTree()
	{
		for (int i = 0; i < m_Children.size(); i++)
		{
			delete m_Children[i];
		}
	}
	

	OcTree(const AABB &oct, const uint32_t size) 
		: m_Oct(oct), m_Size(size) 
	{
		if (size == 1) m_Capacity = 1;
	}

	void AddPoint(const glm::vec3 &point);
	void Generate(const uint32_t &size, const std::vector<glm::vec3> points);

	const uint32_t& GetSize() { return m_Size; }
	const AABB& GetOct() const { return m_Oct; }
	OcTree *GetChild(const int &i) 
	{
		if (i > -1 && i < m_Children.size()) return m_Children[i];
		else return nullptr;
	}
	const std::vector<OcTree*>& GetChildren() const { return m_Children; }
	const std::vector<glm::vec3>& GetPoints() const { return m_Points; }

	int GetPointCount();
	int GetOctCount();
	void GetAllPoints(std::vector<glm::vec3> &points);



private:
	void SubDivide();

private:
	AABB m_Oct;
	uint32_t m_Size = 0;

	size_t m_Capacity = 0;
	bool m_SubDivided = false;

	std::vector<glm::vec3> m_Points = {};
	std::vector<OcTree*> m_Children = {};
};